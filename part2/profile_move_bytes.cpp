#include "repetition_tester.h"
#include "platform_metrics.h"

#include <iostream>

extern "C" void mov_all_bytes(std::uint64_t count, std::uint8_t* dst);
extern "C" void nop_all_bytes(std::uint64_t count, std::uint8_t* dst);

using Test_function = void(*)(std::uint64_t, std::uint8_t*);

struct Test
{
	Test_function function;
	const char* name;
};

int main()
{
	const Test tests[] =
	{
		{mov_all_bytes, "mov_all_bytes"},
		{nop_all_bytes, "nop_all_bytes"}
	};
	constexpr std::uint64_t count = 1024;
	auto data = std::make_unique<std::uint8_t[]>(count);
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
				test.function(count, data.get());
				end_time(tester);
				count_bytes(tester, count);
			}
		}
	}
}
