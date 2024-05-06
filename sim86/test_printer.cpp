#include "printer.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("print mov", "[print]")
{
	{
		std::uint8_t data[2] = {0x89, 0xd9};
		const sim86::PrintResult result = sim86::print(data, data + 2);
		REQUIRE(result.end == data + 2);
		REQUIRE(result.code == "mov cx,bx");
	}
	{
		std::uint8_t data[2] = {0xb1, 0x0c};
		const sim86::PrintResult result = sim86::print(data, data + 2);
		REQUIRE(result.end == data + 2);
		REQUIRE(result.code == "mov cl,0xc");
	}
	{
		std::uint8_t data[3] = {0x8b, 0x56, 0x00};
		const sim86::PrintResult result = sim86::print(data, data + 3);
		REQUIRE(result.end == data + 3);
		REQUIRE(result.code == "mov dx,[bp+0x0]");
	}
	{
		std::uint8_t data[3] = {0x8a, 0x60, 0x04};
		const sim86::PrintResult result = sim86::print(data, data + 3);
		REQUIRE(result.end == data + 3);
		REQUIRE(result.code == "mov ah,[bx+si+0x4]");
	}
	{
		std::uint8_t data[3] = {0xc6, 0x03, 0x07};
		const sim86::PrintResult result = sim86::print(data, data + 3);
		REQUIRE(result.end == data + 3);
		REQUIRE(result.code == "mov byte [bp+di],0x7");
	}
}
