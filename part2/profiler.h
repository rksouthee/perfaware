#pragma once

#include <cstdint>
#include <source_location>
#include <vector>

namespace profiler
{
	struct Section
	{
		std::uint64_t start;
		std::uint64_t end;
		const char* name;
		std::source_location location;
	};

	class Profiler
	{
	private:
		std::vector<Section> m_sections;
		std::uint64_t m_start;
		std::uint64_t m_end;

	public:
		void begin();
		Section& make_section(const char* name, std::source_location location);
		void end();
		void dump();
	};

	extern Profiler g_profiler;

	struct Section_wrapper
	{
		Section* section;
		Section_wrapper(Profiler& profiler, const char* name, std::source_location location);
		Section_wrapper(std::source_location location);
		~Section_wrapper();
	};

#define TIME_BLOCK(name) profiler::Section_wrapper _profile_section##__COUNTER__{name, std::source_location::current()}
#define TIME_FUNCTION profiler::Section_wrapper _profile_section##__COUNTER__{std::source_location::current()}
}
