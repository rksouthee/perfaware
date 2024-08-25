#include "repetition_tester.h"
#include "platform_metrics.h"

#include <format>
#include <iostream>

namespace
{
	double seconds_from_cpu_time(const double cpu_time, const std::uint64_t cpu_timer_frequency)
	{
		double result = 0.0;
		if (cpu_timer_frequency)
		{
			result = cpu_time / static_cast<double>(cpu_timer_frequency);
		}
		return result;
	}

	void print_value(const std::string_view label, const Value value, const std::uint64_t cpu_timer_frequency)
	{
		const std::uint64_t test_count = value[Value_kind::test_count];
		const double denominator = test_count ? static_cast<double>(test_count) : 1.0;

		std::array<double, std::size(value)> normalized_value{};
		for (std::size_t i = 0; i < value_count; ++i)
		{
			normalized_value[i] = static_cast<double>(value[static_cast<Value_kind>(i)]) / denominator;
		}

		std::cout << std::format("{}: {:0f}", label, normalized_value[static_cast<std::size_t>(Value_kind::cpu_timer)]);
		if (cpu_timer_frequency)
		{
			const double seconds = seconds_from_cpu_time(normalized_value[static_cast<std::size_t>(Value_kind::cpu_timer)], cpu_timer_frequency);
			std::cout << " (" << 1000.0f * seconds << "ms)";

			if (normalized_value[static_cast<std::size_t>(Value_kind::byte_count)] > 0)
			{
				constexpr double gigabyte = 1024.0 * 1024.0 * 1024.0;
				const double bandwidth = normalized_value[static_cast<std::size_t>(Value_kind::byte_count)] / (gigabyte * seconds);
				std::cout << ' ' << bandwidth << "gb/s";
			}
		}

		if (normalized_value[static_cast<std::size_t>(Value_kind::page_faults)] > 0)
		{
			std::cout << std::format(" page faults: {:0.4} ({:0.4}k/fault)", normalized_value[static_cast<std::size_t>(Value_kind::page_faults)], normalized_value[static_cast<size_t>(Value_kind::byte_count)] / normalized_value[static_cast<std::size_t>(Value_kind::page_faults)] * 1024.0);
		}
	}

	void print_results(const Test_result& results, const std::uint64_t cpu_timer_frequency)
	{
		print_value("min", results.min, cpu_timer_frequency);
		std::cout << std::endl;
		print_value("max", results.max, cpu_timer_frequency);
		std::cout << std::endl;
		print_value("avg", results.total, cpu_timer_frequency);
		std::cout << std::endl;
	}
}

bool is_testing(Tester& tester)
{
	if (tester.state == Test_state::testing)
	{
		Value accum = tester.accumulate_on_this_test;
		const std::uint64_t current_time = perf::get_cpu_timer();

		if (tester.open_block_count)
		{
			if (tester.open_block_count != tester.closed_block_count)
			{
				error(tester, "open_block_count != close_block_count");
			}

			if (accum[Value_kind::byte_count] != tester.target_processed_byte_count)
			{
				error(tester, "byte_count != target_processed_byte_count");
			}

			if (tester.state == Test_state::testing)
			{
				auto& [total, min, max] = tester.results;
				accum[Value_kind::test_count] = 1;
				for (std::size_t i = 0; i < accum.size(); ++i)
				{
					const auto kind = static_cast<Value_kind>(i);
					total[kind] += accum[kind];
				}

				constexpr auto index = Value_kind::cpu_timer;
				max[index] = std::max(max[index], accum[index]);

				if (accum[index] < min[index])
				{
					min = accum;
					tester.tests_started_at = current_time;
					if (tester.print_new_minimums)
					{
						print_value("min", min, tester.cpu_timer_frequency);
						std::cout << "                                \r";
					}
				}

				tester.open_block_count = 0;
				tester.closed_block_count = 0;
				tester.accumulate_on_this_test.fill(0);
			}
		}

		if (current_time - tester.tests_started_at > tester.try_for_time)
		{
			tester.state = Test_state::finished;
			std::cout << "                                \r";
			print_results(tester.results, tester.cpu_timer_frequency);
		}
	}

	return tester.state == Test_state::testing;
}

void error(Tester& tester, const std::string_view message)
{
	tester.state = Test_state::error;
	std::cerr << "ERROR: " << message << std::endl;
}

void new_test_wave(Tester& tester, const std::uint64_t target_processed_byte_count, const std::uint64_t cpu_timer_frequency, const std::uint32_t seconds_to_try)
{
	if (tester.state == Test_state::none)
	{
		tester.state = Test_state::testing;
		tester.target_processed_byte_count = target_processed_byte_count;
		tester.cpu_timer_frequency = cpu_timer_frequency;
		tester.print_new_minimums = true;
		tester.results.min.fill(std::numeric_limits<std::uint64_t>::max());
	}
	else if (tester.state == Test_state::finished)
	{
		tester.state = Test_state::testing;

		if (tester.target_processed_byte_count != target_processed_byte_count)
		{
			error(tester, "target_processed_byte_count changed");
		}

		if (tester.cpu_timer_frequency != cpu_timer_frequency)
		{
			error(tester, "cpu_timer_frequency changed");
		}
	}

	tester.try_for_time = seconds_to_try * cpu_timer_frequency;
	tester.tests_started_at = perf::get_cpu_timer();
}

void begin_time(Tester& tester)
{
	++tester.open_block_count;
	Value& accumulator = tester.accumulate_on_this_test;
	accumulator[Value_kind::page_faults] -= perf::get_page_fault_count();
	accumulator[Value_kind::cpu_timer] -= perf::get_cpu_timer();
}

void end_time(Tester& tester)
{
	Value& accumulator = tester.accumulate_on_this_test;
	accumulator[Value_kind::page_faults] += perf::get_page_fault_count();
	accumulator[Value_kind::cpu_timer] += perf::get_cpu_timer();
	++tester.closed_block_count;
}

void count_bytes(Tester& tester, const std::uint64_t byte_count)
{
	tester.accumulate_on_this_test[Value_kind::byte_count] += byte_count;
}