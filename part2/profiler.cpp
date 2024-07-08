#include "profiler.h"
#include "platform_metrics.h"

#include <format>
#include <iostream>

namespace profiler
{
	Profiler g_profiler;

	void Profiler::begin()
	{
		m_sections.reserve(4096);
		m_start = perf::get_cpu_timer();
		m_sections.emplace_back(0, 0, 0, 0, "Root", std::source_location::current());
	}

	std::size_t Profiler::make_section(const char* name, std::source_location location)
	{
		(void)m_sections.emplace_back(0, 0, 0, 0, name, location);
		return m_sections.size() - 1;
	}

	Section& Profiler::get_section(std::size_t index)
	{
		return m_sections[index];
	}

	void Profiler::end()
	{
		m_end = perf::get_cpu_timer();
	}

	void Profiler::dump_section(const Section& section, std::uint64_t total_elapsed)
	{
		const float percentage = static_cast<float>(section.elapsed_exclusive) / total_elapsed * 100.0f;
		std::cout << std::format("{}[{}]: {}ms ({:.2f}%", section.name, section.hit_count, perf::ticks_to_ms(section.elapsed_exclusive), percentage);
		if (section.elapsed_inclusive != section.elapsed_exclusive)
		{
			const float percentage_with_children = static_cast<float>(section.elapsed_inclusive) / total_elapsed * 100.0f;
			std::cout << std::format(", {:.2f}% with children", percentage_with_children);
		}

		if (section.bytes_processed != 0)
		{
			const float mb_processed = static_cast<float>(section.bytes_processed) / (1024.0f * 1024.0f);
			const float gb_throughput = static_cast<float>(section.bytes_processed) / (1024.0f * 1024.0f * 1024.0f) / (perf::ticks_to_ms(section.elapsed_exclusive) / 1000.0f);
			std::cout << std::format(", {:.2f}MB at {:.2f}GB/s", mb_processed, gb_throughput);
		}
		std::cout << ')' << std::endl;
	}

	void Profiler::dump()
	{
		const std::uint64_t total_elapsed = m_end - m_start;
		if (total_elapsed != 0)
		{
			for (std::size_t i = 1; i < m_sections.size(); ++i)
			{
				dump_section(m_sections[i], total_elapsed);
			}
		}
	}

	Section_wrapper::Section_wrapper(std::size_t index, std::uint64_t bytes, std::uint64_t start) :
		m_index(index),
		m_parent(g_profiler.get_active_section()),
		m_start(start)
	{
		Section& section = g_profiler.get_section(m_index);
		m_old_elapsed_inclusive = section.elapsed_inclusive;
		section.bytes_processed += bytes;
		g_profiler.set_active_section(m_index);
	}

	Section_wrapper::~Section_wrapper()
	{
		const std::uint64_t elapsed = perf::get_cpu_timer() - m_start;
		g_profiler.set_active_section(m_parent);
		Section& parent_section = g_profiler.get_section(m_parent);
		Section& this_section = g_profiler.get_section(m_index);
		parent_section.elapsed_exclusive -= elapsed;
		this_section.elapsed_exclusive += elapsed;
		this_section.elapsed_inclusive = m_old_elapsed_inclusive + elapsed;
		++this_section.hit_count;
	}
}
