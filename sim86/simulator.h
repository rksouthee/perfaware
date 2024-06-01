#pragma once

#include <cstddef>
#include <cstdint>

namespace sim86
{
	struct Context
	{
		enum Flags
		{
			Flags_zero = 1,
			Flags_sign = 2,
		};
		std::uint8_t memory[0x10000];
		std::uint16_t registers[8];
		std::ptrdiff_t ip;
		Flags flags;
	};

	void execute(const std::uint8_t* first, const std::uint8_t* last, Context& ctx);
}
