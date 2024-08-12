#include "repetition_tester.h"
#include "platform_metrics.h"

#include <iostream>

extern "C" void nop_3x1_all_bytes(std::uint64_t count);
extern "C" void nop_1x3_all_bytes(std::uint64_t count);
extern "C" void nop_1x9_all_bytes(std::uint64_t count);

using Test_function = void(*)(std::uint64_t);

struct Test
{
	Test_function function;
	const char* name;
};

int main()
{
	Test tests[] =
	{
		{nop_3x1_all_bytes, "nop_3x1_all_bytes"},
		{nop_1x3_all_bytes, "nop_1x3_all_bytes"},
		{nop_1x9_all_bytes, "nop_1x9_all_bytes"},
	};
	constexpr std::uint64_t count = 5 * 1024 * 1024;
	Tester testers[std::size(tests)] = {};
	while (true)
	{
		for (std::size_t i = 0; i < std::size(tests); ++i)
		{
			Tester& tester = testers[i];
			const Test test = tests[i];
			std::cout << "\n--- " << test.name << " ---\n";
			new_test_wave(tester, count, perf::get_cpu_timer());
			while (is_testing(tester))
			{
				begin_time(tester);
				test.function(count);
				end_time(tester);
				count_bytes(tester, count);
			}
		}
	}
}
