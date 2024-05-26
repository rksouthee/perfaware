#include "printer.h"
#include "simulator.h"

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

	void write_file(const std::vector<std::uint8_t>& data, std::ostream& os, bool execute)
	{
		os << "bits 16" << std::endl;

		if (data.empty()) return;

		const std::uint8_t* first = &data[0];
		const std::uint8_t* last = first + data.size();
		sim86::Context ctx{};
		do
		{
			const sim86::PrintResult result = sim86::print(first, last);
			if (execute) sim86::execute(first, last, ctx);
			os << result.code << std::endl;
			first = result.end;
		}
		while (first != last);

		if (execute)
		{
			static const char* const s_names[8] = {
				"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"
			};
			for (std::size_t i = 0; i < std::size(ctx.registers); ++i)
			{
				os << s_names[i] << ": " << std::hex << ctx.registers[i] << std::endl;
			}
		}

		const char flags[] = "ZS";
		os << "flags: ";
		for (std::size_t i = 0; i < std::size(flags) - 1; ++i)
		{
			if (ctx.flags & (1 << i))
			{
				os << flags[i];
			}
			else
			{
				os << '-';
			}
		}
		os << std::endl;
	}
}

int main(int argc, char** argv)
{
	cxxopts::Options options("sim86", "Decode an intel instruction set");
	options.add_options()
		("file", "the binary file to decode", cxxopts::value<std::string>())
		("o,output", "write output to file", cxxopts::value<std::string>())
		("execute", "Execute the listing")
		;
	options.parse_positional({ "file" });

	const cxxopts::ParseResult& result = parse(options, argc, argv);
	const std::string& file_name = get_file_name(result);
	std::ifstream file(file_name, std::ios::in | std::ios::binary);
	using I = typename std::istreambuf_iterator<char>;
	const std::vector<std::uint8_t> data(I{file}, I{});

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

	write_file(data, *p_out, result.count("execute"));
	return EXIT_SUCCESS;
}
