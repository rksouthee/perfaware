#pragma once

#include <string>

#include <cstddef>

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
	std::size_t page_faults;
};

bool is_testing(const Tester& tester);
void begin_test(Tester& tester);
void end_test(Tester& tester);
void error(Tester& tester, const std::string& message);
void dump_test_results(const Tester& tester);
