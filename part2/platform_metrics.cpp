#include "platform_metrics.h"

#include <windows.h>
#include <psapi.h>
#include <intrin.h>

namespace perf
{
	std::uint64_t get_os_timer_frequency()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return frequency.QuadPart;
	}

	std::uint64_t get_os_timer()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart;
	}

	std::uint64_t get_cpu_timer()
	{
		return __rdtsc();
	}

	std::uint64_t estimate_cpu_timer_freq()
	{
		std::uint64_t wait_ms = 100;
		std::uint64_t os_frequency = get_os_timer_frequency();
		std::uint64_t cpu_start = get_cpu_timer();
		std::uint64_t os_start = get_os_timer();
		std::uint64_t os_end = 0;
		std::uint64_t os_elapsed = 0;
		std::uint64_t os_wait = os_frequency / 1000 * wait_ms;
		while (os_elapsed < os_wait)
		{
			os_end = get_os_timer();
			os_elapsed = os_end - os_start;
		}
		std::uint64_t cpu_end = get_cpu_timer();
		std::uint64_t cpu_elapsed = cpu_end - cpu_start;
		std::uint64_t cpu_frequency = 0;
		if (os_elapsed)
		{
			cpu_frequency = cpu_elapsed * os_frequency / os_elapsed;
		}
		return cpu_frequency;
	}

	std::uint64_t ticks_to_ms(std::uint64_t ticks, const std::uint64_t cpu_frequency)
	{
		if (cpu_frequency)
		{
			return ticks * 1000 / cpu_frequency;
		}
		return 0;
	}

	double ticks_to_seconds(const std::uint64_t ticks, const std::uint64_t cpu_frequency)
	{
		if (cpu_frequency)
		{
			return static_cast<double>(ticks) / static_cast<double>(cpu_frequency);
		}
		return 0.0;
	}


	std::uint64_t get_page_fault_count()
	{
		PROCESS_MEMORY_COUNTERS memory_info;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &memory_info, sizeof(memory_info)))
		{
			return memory_info.PageFaultCount;
		}
		return 0;
	}
}