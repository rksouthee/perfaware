#include <cassert>
#include <cstdlib>
#include <cstdint>

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <cxxopts.hpp>

namespace
{
	cxxopts::ParseResult parse(cxxopts::Options& options, int argc, char** argv)
	{
		try
		{
			return options.parse(argc, argv);
		}
		catch (const cxxopts::exceptions::parsing& ex)
		{
			std::cerr << ex.what() << std::endl;
			std::cerr << options.help();
			std::exit(EXIT_FAILURE);
		}
	}

	const std::string& get_file_name(const cxxopts::ParseResult& result)
	{
		try
		{
			return result["file"].as<std::string>();
		}
		catch (const cxxopts::exceptions::option_has_no_value&)
		{
			std::cerr << "expected 'file' option" << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}

	void write_file(const std::vector<std::uint8_t>& data, std::ostream& os)
	{
		os << "bits 16" << std::endl;

		auto iter = data.begin();
		while (iter != data.end())
		{
			if (*iter != 0210 && *iter != 0211)
			{
				os << "expected mov (210/211) instruction" << std::endl;
				return;
			}
			os << "mov ";

			const bool is_wide = *iter == 0211;
			++iter;
			const std::uint8_t x = (*iter / 64) % 8;
			const std::uint8_t r = (*iter / 8) % 8;
			const std::uint8_t m = (*iter / 1) % 8;

			const char* registers[2][8] = {
				{ "al", "cl", "dl", "bl", "ah", "ch", "dl", "bh" },
				{ "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" }
			};

			os << registers[is_wide][m];
			os << ", ";
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
					os << registers[is_wide][r];
				}
				break;
			}
			os << std::endl;
			++iter;
		}
	}
}

int main(int argc, char** argv)
{
	cxxopts::Options options("sim86", "Decode an intel instruction set");
	options.add_options()
		("file", "the binary file to decode", cxxopts::value<std::string>())
		("o,output", "write output to file", cxxopts::value<std::string>())
		;
	options.parse_positional({ "file" });

	const cxxopts::ParseResult result = parse(options, argc, argv);
	const std::string& filename = get_file_name(result);
	std::ifstream file(filename, std::ios::in | std::ios::binary);
	using I = typename std::istreambuf_iterator<char>;
	std::vector<std::uint8_t> data(I{ file }, I{});

	std::ofstream outfile;
	std::ostream* p_out;
	if (result.count("output"))
	{
		const std::string& path = result["output"].as<std::string>();
		outfile.open(path);
		p_out = &outfile;
	}
	else
	{
		p_out = &std::cout;
	}

	std::cout << "writing data " << data.size() << std::endl;
	write_file(data, *p_out);
	return EXIT_SUCCESS;
}
