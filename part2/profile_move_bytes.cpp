#include "repetition_tester.h"

#include <iostream>

extern "C" std::uint64_t mov_all_bytes(std::uint64_t count, std::uint8_t* dst);
extern "C" std::uint64_t nop_all_bytes(std::uint64_t count);

using TestFunction = void(*)(std::uint64_t, std::uint8_t*);

void test_mov_all_bytes(std::uint64_t count, std::uint8_t* data)
{
	Tester tester;
	while (is_testing(tester))
	{
		begin_test(tester);
		mov_all_bytes(count, data);
		end_test(tester);
		/* count_bytes(tester, count); */
	}
	dump_test_results(tester);
}

void test_nop_all_bytes(std::uint64_t count, std::uint8_t* data)
{
	Tester tester;
	while (is_testing(tester))
	{
		begin_test(tester);
		nop_all_bytes(count);
		end_test(tester);
		/* count_bytes(tester, count); */
	}
	dump_test_results(tester);
}

struct Test
{
	TestFunction function;
	const char* name;
};

int main()
{
	Test tests[] =
	{
		{test_mov_all_bytes, "mov_all_bytes"},
		{test_nop_all_bytes, "nop_all_bytes"}
	};
	constexpr std::uint64_t count = 1024;
	auto data = std::make_unique<std::uint8_t[]>(count);

	for (const Test& test : tests)
	{
		std::cout << test.name << std::endl;
		test.function(count, data.get());
	}
}
