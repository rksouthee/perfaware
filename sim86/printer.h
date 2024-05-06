#pragma once

#include <cstdint>
#include <string>

namespace sim86
{
	struct PrintResult
	{
		std::string code;
		const std::uint8_t* end;
	};

	PrintResult print(const std::uint8_t* first, const std::uint8_t* last);
}
