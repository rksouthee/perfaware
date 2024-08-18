/*
 * listing_0140_jump_alignment_main.cpp
 */

#include "..\part2\platform_metrics.h"
#include "..\part2\repetition_tester.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <string_view>

extern "C" void nop_aligned_64(std::uint64_t iterations, std::uint8_t* buffer);
extern "C" void nop_aligned_1(std::uint64_t iterations, std::uint8_t* buffer);
extern "C" void nop_aligned_15(std::uint64_t iterations, std::uint8_t* buffer);
extern "C" void nop_aligned_31(std::uint64_t iterations, std::uint8_t* buffer);
extern "C" void nop_aligned_63(std::uint64_t iterations, std::uint8_t* buffer);

namespace
{
	using Function = void (*) (std::uint64_t, std::uint8_t*);

	struct Test_function
	{
		std::string_view name;
		Function function;
	};

	const Test_function s_test_functions[] =
	{
	{ "nop_aligned_64", nop_aligned_64 },
	{ "nop_aligned_1", nop_aligned_1 },
	{ "nop_aligned_15", nop_aligned_15 },
	{ "nop_aligned_31", nop_aligned_31 },
	{ "nop_aligned_63", nop_aligned_63 }
	};
}

int main()
{
	const std::size_t buffer_size = 1 * 1024 * 1024 * 1024;
	auto buffer = std::make_unique<std::uint8_t[]>(buffer_size);

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
			Test_function function = s_test_functions[i];

			std::cout << "\n--- " << function.name << " ---" << std::endl;
			const std::uint64_t cpu_timer_frequency = perf::estimate_cpu_timer_freq();
			new_test_wave(tester, buffer_size, cpu_timer_frequency);

			while (is_testing(tester))
			{
				begin_time(tester);
				function.function(buffer_size, buffer.get());
				end_time(tester);
				count_bytes(tester, buffer_size);
			}
		}
	}
}
