#include <cassert>
#include <cstdint>

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <cxxopts.hpp>

int main(int argc, char** argv)
{
	cxxopts::Options options("sim86", "Decode an intel instruction set");
	options.add_options()
		("file", "the binary file to decode", cxxopts::value<std::string>());
	options.parse_positional({ "file" });
	const cxxopts::ParseResult result = options.parse(argc, argv);
	const auto& filename = result["file"].as<std::string>();
	std::ifstream file(filename, std::ios::in | std::ios::binary);
	using I = typename std::istreambuf_iterator<char>;
	std::vector<std::uint8_t> data(I{ file }, I{});

	std::cout << "bits 16" << std::endl;

	auto iter = data.begin();
	while (iter != data.end())
	{
		if (*iter != 0210 && *iter != 0211)
		{
			std::cout << "expected mov (210/211) instruction" << std::endl;
			return 0;
		}
		std::cout << "mov ";

		const bool is_wide = *iter == 0211;
		++iter;
		const std::uint8_t x = (*iter / 64) % 8;
		const std::uint8_t r = (*iter / 8) % 8;
		const std::uint8_t m = (*iter / 1) % 8;

		const char* registers[2][8] = {
			{ "al", "cl", "dl", "bl", "ah", "ch", "dl", "bh" },
			{ "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" }
		};

		std::cout << registers[is_wide][m];
		std::cout << ", ";
		assert(x <= 3);
		switch (x)
		{
		case 0:
		{
		}
		break;

		case 1:
		{
		}
		break;

		case 2:
		{
		}
		break;

		case 3:
		{
			std::cout << registers[is_wide][r];
		}
		break;
		}
		std::cout << std::endl;
		++iter;
	}

	return 0;
}
