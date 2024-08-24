/*
 * Listing 145
 */

#include "..\part2\platform_metrics.h"
#include "..\part2\repetition_tester.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <string_view>

extern "C" void read_x1(std::uint64_t iterations, std::uint8_t* buffer);
extern "C" void read_x2(std::uint64_t iterations, std::uint8_t* buffer);
extern "C" void read_x3(std::uint64_t iterations, std::uint8_t* buffer);
extern "C" void read_x4(std::uint64_t iterations, std::uint8_t* buffer);

namespace
{
	using Function = void (*) (std::uint64_t, std::uint8_t*);

	struct Test_function
	{
		std::string_view name;
		Function function;
	};

	constexpr Test_function s_test_functions[] =
	{
	{ "read_x1", read_x1 },
	{ "read_x2", read_x2 },
	{ "read_x3", read_x3 },
	{ "read_x4", read_x4 },
	};
}

[[noreturn]] int main()
{
	constexpr std::size_t buffer_size = 1 * 1024 * 1024 * 1024;
	const auto buffer = std::make_unique<std::uint8_t[]>(buffer_size);

	if (!buffer)
	{
		std::cerr << "Failed to allocate buffer" << std::endl;
		return EXIT_FAILURE;
	}

	Tester testers[std::size(s_test_functions)] = {};
	while (true)
	{
		for (std::size_t i = 0; i < std::size(s_test_functions); ++i)
		{
			Tester& tester = testers[i];
			auto [name, function] = s_test_functions[i];

			std::cout << "\n--- " << name << " ---" << std::endl;
			const std::uint64_t cpu_timer_frequency = perf::estimate_cpu_timer_freq();
			new_test_wave(tester, buffer_size, cpu_timer_frequency);

			while (is_testing(tester))
			{
				begin_time(tester);
				function(buffer_size, buffer.get());
				end_time(tester);
				count_bytes(tester, buffer_size);
			}
		}
	}
}
