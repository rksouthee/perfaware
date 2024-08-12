#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include "repetition_tester.h"
#include "platform_metrics.h"

#include <iostream>
#include <string>

#include <cstdio>

#include <windows.h>

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
			begin_time(tester);
			while (total_bytes_read < params.buffer.size &&
			       ReadFile(handle, &ptr[total_bytes_read], params.buffer.size - total_bytes_read, &read_bytes, NULL))
			{
				total_bytes_read += read_bytes;
			}
			end_time(tester);
			count_bytes(tester, total_bytes_read);
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
			begin_time(tester);
			std::size_t count = std::fread(params.buffer.data, params.buffer.size, 1, stream);
			end_time(tester);
			count_bytes(tester, count);
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
	Tester testers[std::size(s_read_fns)][std::size(allocation_types)] = {};
	for (std::size_t i = 0; i < std::size(s_read_fns); ++i)
	{
		const Read_fn fn = s_read_fns[i];
		for (std::size_t j = 0; j < std::size(allocation_types); ++j)
		{
			const Allocation_type allocation_type = allocation_types[j];
			Tester& tester = testers[i][j];
			new_test_wave(tester, file_size, perf::get_cpu_timer());
			Read_parameters params{path, make_buffer(allocation_type, file_size), allocation_type};
			fn(tester, params);
			free_buffer(params.buffer, allocation_type);
		}
	}
}
