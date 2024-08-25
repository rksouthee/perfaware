#include "..\part2\platform_metrics.h"
#include "..\part2\repetition_tester.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <numeric>
#include <string_view>
#include <windows.h>

extern "C" void process_bytes(std::uint64_t buffer_size, std::uint8_t* buffer, std::uint64_t mask);

namespace
{
	constexpr std::size_t buffer_size = 1 * 1024 * 1024 * 1024;

	void run(const std::unique_ptr<std::uint8_t[]>& buffer)
	{
		std::iota(buffer.get(), buffer.get() + buffer_size, 0);
		Tester testers[30] = {};
		const std::uint64_t cpu_timer_frequency = perf::estimate_cpu_timer_freq();
		constexpr std::size_t min_index = 10;
		for (std::size_t i = min_index; i < std::size(testers); ++i)
		{
			Tester& tester = testers[i];

			const std::size_t bytes = 1 << i;
			std::cout << "\n--- " << bytes << " ---" << std::endl;
			new_test_wave(tester, buffer_size, cpu_timer_frequency);

			while (is_testing(tester))
			{
				begin_time(tester);
				process_bytes(buffer_size, buffer.get(), bytes - 1);
				end_time(tester);
				count_bytes(tester, buffer_size);
			}
		}

		// Output csv data
		for (std::size_t i = min_index; i < std::size(testers); ++i)
		{
			const Tester& tester = testers[i];
			const Value& value = tester.results.min;
			const auto seconds = perf::ticks_to_seconds(value[Value_kind::cpu_timer], cpu_timer_frequency);
			constexpr double gigabyte = 1024.0 * 1024.0 * 1024.0;
			const double bandwidth = static_cast<double>(value[Value_kind::byte_count]) / (gigabyte * seconds);
			std::cout << (1 << i) << ',' << bandwidth << std::endl;
		}
	}
}

int main()
{
	const auto buffer = std::make_unique<std::uint8_t[]>(buffer_size);

	if (!buffer)
	{
		std::cerr << "Failed to allocate buffer" << std::endl;
		return EXIT_FAILURE;
	}

	run(buffer);
}