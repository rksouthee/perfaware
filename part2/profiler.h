#pragma once

#include "platform_metrics.h"

#include <cstdint>
#include <source_location>
#include <vector>

namespace profiler
{
	struct Section
	{
		std::uint64_t elapsed_exclusive;
		std::uint64_t elapsed_inclusive;
		std::uint64_t bytes_processed;
		std::uint64_t hit_count;
		const char* name;
		std::source_location location;
	};

	class Profiler
	{
	private:
		std::vector<Section> m_sections;
		std::uint64_t m_start;
		std::uint64_t m_end;
		std::size_t m_current_section = 0;

		void dump_section(const Section& section, std::uint64_t total_elapsed);

	public:
		void begin();
		std::size_t make_section(const char* name, std::source_location location);
		void end();
		void dump();

		Section& get_section(std::size_t index);

		std::size_t get_active_section() const { return m_current_section; }
		void set_active_section(std::size_t index) { m_current_section = index; }
	};

	extern Profiler g_profiler;

	class Section_wrapper
	{
	private:
		std::size_t m_index;
		std::size_t m_parent;
		std::uint64_t m_start;
		std::uint64_t m_old_elapsed_inclusive;

	public:
		Section_wrapper(std::size_t index, std::uint64_t bytes, std::uint64_t start);
		~Section_wrapper();
	};
}

#if PROFILER
#define TIME_BLOCK_(name, bytes, counter) \
	static std::size_t s_profile_section_counter_##counter = profiler::g_profiler.make_section(name, std::source_location::current());\
	const profiler::Section_wrapper _profile_section_wrapper_##counter{s_profile_section_counter_##counter, bytes, perf::get_cpu_timer()}
#define TIME_BYTES_PROCESSED(name, bytes) TIME_BLOCK_(name, bytes, __COUNTER__)
#define TIME_BLOCK(name) TIME_BYTES_PROCESSED(name, 0)
#define TIME_FUNCTION TIME_BLOCK(__FUNCTION__)
#else
#define TIME_BYTES_PROCESSED(name, bytes)
#define TIME_BLOCK(name)
#define TIME_FUNCTION
#endif
