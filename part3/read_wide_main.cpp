/*
 * Listing 151
 */

#include "..\part2\platform_metrics.h"
#include "..\part2\repetition_tester.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <string_view>
#include <windows.h>

extern "C" void read_4x3(std::uint64_t iterations, std::uint8_t *buffer);

extern "C" void read_8x3(std::uint64_t iterations, std::uint8_t *buffer);

extern "C" void read_16x3(std::uint64_t iterations, std::uint8_t *buffer);

extern "C" void read_32x3(std::uint64_t iterations, std::uint8_t *buffer);

namespace
{
	using Function = void (*)(std::uint64_t, std::uint8_t *);

	struct Test_function
	{
		std::string_view name;
		Function function;
	};

	constexpr Test_function s_test_functions[] = {
		{"read_4x3", read_4x3},
		{"read_8x3", read_8x3},
		{"read_16x3", read_16x3},
		{"read_32x3", read_32x3},
	};

	[[noreturn]] void run(const std::unique_ptr<std::uint8_t[]> &buffer, const std::size_t buffer_size)
	{
		const std::uint64_t cpu_timer_frequency = perf::estimate_cpu_timer_freq();
		Tester testers[std::size(s_test_functions)] = {};
		while (true)
		{
			for (std::size_t i = 0; i < std::size(s_test_functions); ++i)
			{
				Tester &tester = testers[i];
				auto [name, function] = s_test_functions[i];

				std::cout << "\n--- " << name << " ---" << std::endl;
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
}

int main()
{
	constexpr std::size_t buffer_size = 1 * 1024 * 1024 * 1024;
	const auto buffer = std::make_unique<std::uint8_t[]>(buffer_size);

	if (!buffer)
	{
		std::cerr << "Failed to allocate buffer" << std::endl;
		return EXIT_FAILURE;
	}

	run(buffer, buffer_size);
}
