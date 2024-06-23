#include "profiler.h"
#include "platform_metrics.h"

#include <format>
#include <iostream>

namespace profiler
{
	Profiler g_profiler;

	void Profiler::begin()
	{
		m_start = perf::get_cpu_timer();
	}

	Section& Profiler::make_section(const char* name, std::source_location location)
	{
		return m_sections.emplace_back(perf::get_cpu_timer(), 0, name, location);
	}

	void Profiler::end()
	{
		m_end = perf::get_cpu_timer();
	}

	void Profiler::dump()
	{
		const std::uint64_t total_elapsed = m_end - m_start;
		for (const Section& section : m_sections)
		{
			const std::uint64_t elpased = section.end - section.start;
			const float percentage = static_cast<float>(elpased) / total_elapsed * 100.0f;
			std::cout << std::format("{}: {}ms ({:.2f}%)\n", section.name, perf::ticks_to_ms(elpased), percentage);
		}
	}

	Section_wrapper::Section_wrapper(Profiler& profiler, const char* name, std::source_location location)
	{
		section = &profiler.make_section(name, location);
	}

	Section_wrapper::Section_wrapper(std::source_location location) : Section_wrapper{ g_profiler, location.function_name(), location }
	{
	}

	Section_wrapper::~Section_wrapper()
	{
		section->end = perf::get_cpu_timer();
	}
}
