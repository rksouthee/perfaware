#pragma once

#include <cstdint>

namespace perf
{
	std::uint64_t get_os_timer_frequency();
	std::uint64_t get_os_timer();
	std::uint64_t get_cpu_timer();
	std::uint64_t estimate_cpu_timer_freq();
	std::uint64_t ticks_to_ms(std::uint64_t ticks, std::uint64_t cpu_frequency);
	double ticks_to_seconds(std::uint64_t ticks, std::uint64_t cpu_frequency);
	std::uint64_t get_page_fault_count();
}