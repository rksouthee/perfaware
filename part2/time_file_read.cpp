#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <cstdio>
#include <windows.h>
#include "platform_metrics.h"
#include <iostream>
#include <string>

// ReadFile
// fread
// ifstream

enum class Tester_state
{
	testing,
	error,
	finished,
};

struct Tester
{
	Tester_state state;
	std::size_t min_time;
	std::size_t start_time;
	std::size_t total_time;
	std::size_t iterations;
	std::size_t time_since_last_min;
	std::size_t max_time_between_mins;
	std::size_t page_faults;
};

bool is_testing(const Tester& tester)
{
	return tester.state == Tester_state::testing;
}

void begin_test(Tester& tester)
{
	tester.start_time = perf::get_cpu_timer();
	tester.page_faults -= perf::get_page_fault_count();
}

void end_test(Tester& tester)
{
	++tester.iterations;
	tester.page_faults += perf::get_page_fault_count();
	if (tester.state != Tester_state::testing)
	{
		return;
	}
	std::uint64_t cpu_time = perf::get_cpu_timer();
	std::uint64_t elapsed = cpu_time - tester.start_time;
	std::uint64_t elapsed_ms = perf::ticks_to_ms(elapsed);
	if (tester.min_time == 0 || elapsed_ms < tester.min_time)
	{
		tester.min_time = elapsed_ms;
		tester.time_since_last_min = 0;
	}
	else
	{
		tester.time_since_last_min += elapsed_ms;
		if (tester.time_since_last_min > tester.max_time_between_mins)
		{
			tester.state = Tester_state::finished;
		}
	}
}

void error(Tester& tester, const std::string& message)
{
	std::cerr << message << std::endl;
	tester.state = Tester_state::error;
}

void dump_test_results(const Tester& tester)
{
	std::cout << "min time: " << tester.min_time << " ms";
	if (tester.page_faults > 0)
	{
		std::cout << " page faults: " << tester.page_faults;
	}
	std::cout << std::endl;
}

struct Buffer
{
	void* data;
	std::size_t size;
};

enum class Allocation_type
{
	none,
	allocate,
};

struct Read_parameters
{
	const std::string& path;
	Buffer buffer;
	Allocation_type allocation_type;
};

void handle_allocation(Read_parameters& params)
{
	switch (params.allocation_type)
	{
	case Allocation_type::none:
		{
			break;
		}
	case Allocation_type::allocate:
		{
			params.buffer.data = std::malloc(params.buffer.size);
			break;
		}
	}
}

void handle_deallocation(Read_parameters& params)
{
	switch (params.allocation_type)
	{
	case Allocation_type::none:
		{
			break;
		}
	case Allocation_type::allocate:
		{
			std::free(params.buffer.data);
			break;
		}
	}
}

const char* describe_allocation_type(Allocation_type type)
{
	switch (type)
	{
	case Allocation_type::none:
		{
			return "none";
		}
	case Allocation_type::allocate:
		{
			return "allocate";
		}
	}
	return "unknown";
}

void test_win32_read_file(Tester& tester, Read_parameters& params)
{
	std::cout << "test_win32_read_file " << describe_allocation_type(params.allocation_type) << std::endl;
	while (is_testing(tester))
	{
		HANDLE handle = CreateFileA(params.path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle)
		{
			handle_allocation(params);
			DWORD total_bytes_read = 0;
			DWORD read_bytes = 0;
			auto ptr = (unsigned char*)params.buffer.data;
			begin_test(tester);
			while (total_bytes_read < params.buffer.size &&
			       ReadFile(handle, &ptr[total_bytes_read], params.buffer.size - total_bytes_read, &read_bytes, NULL))
			{
				total_bytes_read += read_bytes;
			}
			end_test(tester);
			if (total_bytes_read < params.buffer.size)
			{
				error(tester, "failed to read file");
			}
			handle_deallocation(params);
			CloseHandle(handle);
		}
		else
		{
			error(tester, "failed to open file");
		}
	}
}

void test_fread(Tester& tester, Read_parameters& params)
{
	std::cout << "test_fread " << describe_allocation_type(params.allocation_type) << std::endl;
	while (is_testing(tester))
	{
		std::FILE* stream = std::fopen(params.path.c_str(), "rb");
		if (stream)
		{
			handle_allocation(params);
			begin_test(tester);
			std::size_t count = std::fread(params.buffer.data, params.buffer.size, 1, stream);
			end_test(tester);
			if (count != 1)
			{
				error(tester, "failed to read file");
			}
			handle_deallocation(params);
			std::fclose(stream);
		}
		else
		{
			error(tester, "failed to open file");
		}
	}
}

using Read_fn = void(*)(Tester& tester, Read_parameters& params);

const Read_fn s_read_fns[] =
{
	test_win32_read_file,
	test_fread,
};

std::uint64_t get_file_size(const std::string& path)
{
	std::uint64_t result = 0;
	HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle)
	{
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(handle, &file_size))
		{
			result = file_size.QuadPart;
		}
		CloseHandle(handle);
	}
	return result;
}

Buffer make_buffer(Allocation_type allocation_type, const std::size_t size)
{
	Buffer buffer{};
	buffer.size = size;
	switch (allocation_type)
	{
	case Allocation_type::none:
		{
			if (buffer.size > 0)
			{
				buffer.data = std::malloc(buffer.size);
			}
			break;
		}
	case Allocation_type::allocate:
		{
			break;
		}
	}
	return buffer;
}

void free_buffer(const Buffer& buffer, Allocation_type allocation_type)
{
	switch (allocation_type)
	{
	case Allocation_type::none:
		{
			std::free(buffer.data);
			break;
		}
	case Allocation_type::allocate:
		{
			break;
		}
	}
}

int main()
{
	static const Allocation_type allocation_types[] = {Allocation_type::none, Allocation_type::allocate};
	const std::string path = "haversine_data.json";
	const std::size_t file_size = get_file_size(path);
	for (const auto& fn : s_read_fns)
	{
		for (const Allocation_type allocation_type : allocation_types)
		{
			Read_parameters params{path, make_buffer(allocation_type, file_size), allocation_type};
			Tester tester{};
			tester.max_time_between_mins = 10'000;
			fn(tester, params);
			dump_test_results(tester);
			free_buffer(params.buffer, allocation_type);
		}
	}
}
