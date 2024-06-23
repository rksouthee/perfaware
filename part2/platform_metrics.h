#include <cstdint>

namespace perf
{
	std::uint64_t get_os_timer_frequency();
	std::uint64_t get_os_timer();
	std::uint64_t get_cpu_timer();
	std::uint64_t estimate_cpu_timer_freq();
	std::uint64_t ticks_to_ms(std::uint64_t ticks);
}
