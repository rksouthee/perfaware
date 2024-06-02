#include "printer.h"
#include "simulator.h"

#include <cstdlib>
#include <cstdint>

#include <format>
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

	void write_file(const std::vector<std::uint8_t>& data, std::ostream& os, const cxxopts::ParseResult& options)
	{
		os << "bits 16" << std::endl;

		if (data.empty()) return;
		if (data.size() > sizeof(sim86::Context::memory))
		{
			std::cerr << "file too large" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		sim86::Context ctx{};
		const std::uint8_t* last = std::copy(data.begin(), data.end(), ctx.memory);
		while (static_cast<std::size_t>(ctx.ip) < data.size())
		{
			const std::uint8_t* first = &ctx.memory[ctx.ip];
			const sim86::PrintResult result = sim86::print(first, last);
			ctx.ip += result.end - first;
			std::string comment;
			if (options.count("execute"))
			{
				sim86::execute(first, result.end, ctx);
				if (options.count("showclocks"))
				{
					if (ctx.clocks == 0) os << " no clocks for " << std::hex << (int)first[0] << std::endl;
					comment = std::format("Clocks: {:+d} = {:d}", ctx.clocks, ctx.total_clocks);
				}
			}

			os << result.code;
			if (!comment.empty())
			{
				os << " ; " << comment;
			}
			os << std::endl;
		}

		if (options.count("execute"))
		{
			std::cout << "ip: " << std::hex << ctx.ip << std::endl;
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

		if (options.count("dump"))
		{
			std::ofstream dump_file("dump.data", std::ios::out | std::ios::binary);
			dump_file.write(reinterpret_cast<const char*>(ctx.memory), std::size(ctx.memory));
		}
	}
}

int main(int argc, char** argv)
{
	cxxopts::Options options("sim86", "Decode an intel instruction set");
	options.add_options()
		("file", "the binary file to decode", cxxopts::value<std::string>())
		("o,output", "write output to file", cxxopts::value<std::string>())
		("execute", "Execute the listing")
		("dump", "Dump the memory to a file")
		("showclocks", "Show the number of clocks taken")
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

	write_file(data, *p_out, result);
	return EXIT_SUCCESS;
}
