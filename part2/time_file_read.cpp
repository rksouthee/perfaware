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
};

bool is_testing(const Tester& tester)
{
	return tester.state == Tester_state::testing;
}

void begin_test(Tester& tester)
{
	tester.start_time = perf::get_cpu_timer();
}

void end_test(Tester& tester)
{
	++tester.iterations;
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
	std::cout << "min time: " << tester.min_time << " ms" << std::endl;
}

struct Read_parameters
{
	const std::string& path;
	std::string& buffer;
};

void test_win32_read_file(Tester& tester, const Read_parameters& params)
{
	std::cout << "test_win32_read_file" << std::endl;
	while (is_testing(tester))
	{
		begin_test(tester);
		HANDLE handle = CreateFileA(params.path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle)
		{
			DWORD total_bytes_read = 0;
			DWORD read_bytes = 0;
			while (total_bytes_read < params.buffer.size() &&
			       ReadFile(handle, (void*)&params.buffer[total_bytes_read], params.buffer.size() - total_bytes_read, &read_bytes, NULL))
			{
				total_bytes_read += read_bytes;
			}
			if (total_bytes_read < params.buffer.size())
			{
				error(tester, "failed to read file");
			}
			CloseHandle(handle);
		}
		else
		{
			error(tester, "failed to open file");
		}
		end_test(tester);
	}
}

void test_fread(Tester& tester, const Read_parameters& params)
{
	std::cout << "test_fread" << std::endl;
	while (is_testing(tester))
	{
		begin_test(tester);
		std::FILE* stream = std::fopen(params.path.c_str(), "rb");
		if (stream)
		{
			std::size_t count = std::fread((void*)params.buffer.data(), params.buffer.size(), 1, stream);
			if (count != 1)
			{
				error(tester, "failed to read file");
			}
			std::fclose(stream);
		}
		else
		{
			error(tester, "failed to open file");
		}
		end_test(tester);
	}
}

using Read_fn = void(*)(Tester& tester, const Read_parameters& params);

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

int main()
{
	const std::string path = "haversine_data.json";
	std::string buffer(get_file_size(path), '\0');
	for (const auto& fn : s_read_fns)
	{
		Tester tester{};
		tester.max_time_between_mins = 10'000;
		const Read_parameters params{path, buffer};
		fn(tester, params);
		dump_test_results(tester);
	}
}
