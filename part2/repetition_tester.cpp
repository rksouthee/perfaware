#include "repetition_tester.h"
#include "platform_metrics.h"

#include <iostream>

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

