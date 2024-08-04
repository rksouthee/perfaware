#include "repetition_tester.h"

#include <iostream>

extern "C" void nop_3x1_all_bytes(std::uint64_t count);
extern "C" void nop_1x3_all_bytes(std::uint64_t count);
extern "C" void nop_1x9_all_bytes(std::uint64_t count);

using TestFunction = void(*)(std::uint64_t);

struct Test
{
	TestFunction function;
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
	for (const Test& test : tests)
	{
		std::cout << test.name << std::endl;
		Tester tester{};
		while (is_testing(tester))
		{
			begin_test(tester);
			test.function(count);
			end_test(tester);
		}
		dump_test_results(tester);
	}
}
