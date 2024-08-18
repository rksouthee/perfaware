#pragma once

#include <array>
#include <string_view>

#include <cstddef>
#include <cstdint>

enum class Test_state
{
	none,
	testing,
	error,
	finished,
};

enum class Value_kind
{
	test_count,
	cpu_timer,
	page_faults,
	byte_count
};

constexpr std::size_t value_count = 4;

using Value = std::array<std::uint64_t, value_count>;

struct Test_result
{
	Value total;
	Value min;
	Value max;
};

struct Tester
{
	std::uint64_t target_processed_byte_count;
	std::uint64_t cpu_timer_frequency;
	std::uint64_t try_for_time;
	std::uint64_t tests_started_at;

	Test_state state;
	bool print_new_minimums;
	std::uint32_t open_block_count;
	std::uint32_t closed_block_count;

	Value accumulate_on_this_test;
	Test_result results;
};

void error(Tester& tester, std::string_view message);
void new_test_wave(Tester& tester, std::uint64_t target_processed_byte_count, std::uint64_t cpu_timer_frequency, std::uint32_t seconds_to_try = 10);
bool is_testing(Tester& tester);
void begin_time(Tester& tester);
void end_time(Tester& tester);
void count_bytes(Tester& tester, std::uint64_t byte_count);
