#pragma once

#include <cstddef>
#include <cstdint>

namespace sim86
{
	struct Context
	{
		std::uint16_t registers[8];
		std::ptrdiff_t ip;
	};

	void execute(const std::uint8_t* first, const std::uint8_t* last, Context& ctx);
}
