#include "platform_metrics.h"
#include <Windows.h>
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: count_page_faults <page_count>" << std::endl;
		return -1;
	}

	std::size_t page_size = 4096;
	std::size_t page_count = std::atol(argv[1]);
	std::size_t total_size = page_size * page_count;

	for (std::uint64_t i = 0; i < page_count; ++i)
	{
		auto ptr = (unsigned char*)VirtualAlloc(nullptr, total_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!ptr)
		{
			std::cerr << "VirtualAlloc failed" << std::endl;
			return -1;
		}
		std::uint64_t count_begin = perf::get_page_fault_count();
		for (std::size_t j = 0; j < i * page_size; ++j)
		{
			ptr[j] = (unsigned char)j;
		}
		std::uint64_t count_end = perf::get_page_fault_count();
		std::uint64_t count = count_end - count_begin;
		std::cout << page_count << ", " << i << ", " << count << ", " <<  count - i << std::endl;
		VirtualFree(ptr, 0, MEM_RELEASE);
	}
	return 0;
}
