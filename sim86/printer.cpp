#include "printer.h"

#include <format>
#include <optional>
#include <sstream>

namespace
{
	const char* const s_wide_registers[8] =
	{
		"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"
	};

	const char* const s_byte_registers[8] =
	{
		"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"
	};

	const char* const s_ea_registers[8] =
	{
		"bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx"
	};

	const char* get_register(const std::size_t reg, const bool w)
	{
		if (w) return s_wide_registers[reg];
		return s_byte_registers[reg];
	}

	struct Mod_reg_rm
	{
		std::string reg;
		std::string r_m;
		const std::uint8_t* end;
	};

	std::optional<Mod_reg_rm> parse_mod_reg_rm(const std::uint8_t* first, const std::uint8_t* last, bool w)
	{
		if (first == last) return std::nullopt;
		const std::uint8_t value = *first++;
		const std::uint8_t r_m = value & 0x7;
		const std::uint8_t reg = (value >> 3) & 0x7;
		const std::uint8_t mod = (value >> 6) & 0x3;

		Mod_reg_rm result;
		switch (mod)
		{
		case 0b00:
			{
				if (r_m == 0b110)
				{
					if (last - first < 2) return std::nullopt;
					const std::uint16_t address = first[0] | (first[1] << 8);
					result.r_m = std::format("[{:#x}]", address);
					first += 2;
				}
				else
				{
					result.r_m = std::format("[{}]", s_ea_registers[r_m]);
				}
			}
			break;
		case 0b01:
			{
				if (last - first < 1) return std::nullopt;
				result.r_m = std::format("[{}{:+#x}]", s_ea_registers[r_m], static_cast<int>(static_cast<std::int8_t>(first[0])));
				++first;
			}
			break;
		case 0b10:
			{
				if (last - first < 2) return std::nullopt;
				const std::int16_t address = first[0] | (first[1] << 8);
				result.r_m = std::format("[{}{:+#x}]", s_ea_registers[r_m], address);
				first += 2;
			}
			break;
		case 0b11:
			{
				result.r_m = get_register(r_m, w);
			}
			break;
		}
		result.reg = get_register(reg, w);
		result.end = first;
		return result;
	}

#define PRINT_FN(name) sim86::PrintResult name(const std::uint8_t* first, const std::uint8_t* last)
	typedef PRINT_FN((*Print_fn));

	PRINT_FN(bytes)
	{
		std::ostringstream oss;
		oss << "db 0x";
		while (first != last) oss << std::hex << static_cast<int>(*first++);
		return { oss.str(), last };
	}

	sim86::PrintResult mov_rm_reg(const std::uint8_t* first, const std::uint8_t* last, const bool w, const bool d)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::optional<Mod_reg_rm> result = parse_mod_reg_rm(first, last, w);
		if (!result) return bytes(first - 1, last);
		std::ostringstream oss;
		oss << "mov ";
		if (d) oss << result->reg << ',' << result->r_m;
		else oss << result->r_m << ',' << result->reg;
		return { oss.str(), result->end };
	}

	sim86::PrintResult mov_reg_immed_8(const std::uint8_t* first, const std::uint8_t* last, const char* const reg)
	{
		if (last - first < 2) return bytes(first, last);
		return { std::format("mov {},{:#x}", reg, static_cast<int>(first[1])), first + 2 };
	}

	sim86::PrintResult mov_reg_immed_16(const std::uint8_t* first, const std::uint8_t* last, const char* const reg)
	{
		if (last - first < 3) return bytes(first, last);
		const std::uint16_t immed = first[1] | (first[2] << 8);
		return { std::format("mov {},{:#x}", reg, immed), first + 3 };
	}

	PRINT_FN(mov_cl_immed)
	{
		return mov_reg_immed_8(first, last, "cl");
	}

	PRINT_FN(mov_ch_immed)
	{
		return mov_reg_immed_8(first, last, "ch");
	}

	PRINT_FN(mov_ax_immed)
	{
		return mov_reg_immed_16(first, last, "ax");
	}

	PRINT_FN(mov_cx_immed)
	{
		return mov_reg_immed_16(first, last, "cx");
	}

	PRINT_FN(mov_dx_immed)
	{
		return mov_reg_immed_16(first, last, "dx");
	}

	PRINT_FN(mov_mem_immed_8)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::optional<Mod_reg_rm> result = parse_mod_reg_rm(first, last, 0);
		if (!result || last - result->end < 1) return bytes(first - 1, last);
		return { std::format("mov byte {},{:#x}", result->r_m, static_cast<int>(result->end[0])), result->end + 1 };
	}

	PRINT_FN(mov_mem_immed_16)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::optional<Mod_reg_rm> result = parse_mod_reg_rm(first, last, 1);
		if (!result || last - result->end < 2) return bytes(first - 1, last);
		const std::uint16_t immed = result->end[0] | (result->end[1] << 8);
		return { std::format("mov word {},{:#x}", result->r_m, immed), result->end + 2 };
	}

	PRINT_FN(mov_rm_reg_8)
	{
		return mov_rm_reg(first, last, 0, 0);
	}

	PRINT_FN(mov_rm_reg_16)
	{
		return mov_rm_reg(first, last, 1, 0);
	}

	PRINT_FN(mov_reg_rm_8)
	{
		return mov_rm_reg(first, last, 0, 1);
	}

	PRINT_FN(mov_reg_rm_16)
	{
		return mov_rm_reg(first, last, 1, 1);
	}

	PRINT_FN(mov_ax_mem)
	{
		if (last - first < 3) return bytes(first, last);
		const std::uint16_t addr = first[1] | (first[2] << 8);
		return { std::format("mov ax,[{:#x}]", addr), first + 3 };
	}

	PRINT_FN(mov_mem_ax)
	{
		if (last - first < 3) return bytes(first, last);
		const std::uint16_t addr = first[1] | (first[2] << 8);
		return { std::format("mov [{:#x}],ax", addr), first + 3 };
	}

	const Print_fn s_printers[256] =
	{
		/* 0x00 */ bytes,
		/* 0x01 */ bytes,
		/* 0x02 */ bytes,
		/* 0x03 */ bytes,
		/* 0x04 */ bytes,
		/* 0x05 */ bytes,
		/* 0x06 */ bytes,
		/* 0x07 */ bytes,
		/* 0x08 */ bytes,
		/* 0x09 */ bytes,
		/* 0x0a */ bytes,
		/* 0x0b */ bytes,
		/* 0x0c */ bytes,
		/* 0x0d */ bytes,
		/* 0x0e */ bytes,
		/* 0x0f */ bytes,
		/* 0x10 */ bytes,
		/* 0x11 */ bytes,
		/* 0x12 */ bytes,
		/* 0x13 */ bytes,
		/* 0x14 */ bytes,
		/* 0x15 */ bytes,
		/* 0x16 */ bytes,
		/* 0x17 */ bytes,
		/* 0x18 */ bytes,
		/* 0x19 */ bytes,
		/* 0x1a */ bytes,
		/* 0x1b */ bytes,
		/* 0x1c */ bytes,
		/* 0x1d */ bytes,
		/* 0x1e */ bytes,
		/* 0x1f */ bytes,
		/* 0x20 */ bytes,
		/* 0x21 */ bytes,
		/* 0x22 */ bytes,
		/* 0x23 */ bytes,
		/* 0x24 */ bytes,
		/* 0x25 */ bytes,
		/* 0x26 */ bytes,
		/* 0x27 */ bytes,
		/* 0x28 */ bytes,
		/* 0x29 */ bytes,
		/* 0x2a */ bytes,
		/* 0x2b */ bytes,
		/* 0x2c */ bytes,
		/* 0x2d */ bytes,
		/* 0x2e */ bytes,
		/* 0x2f */ bytes,
		/* 0x30 */ bytes,
		/* 0x31 */ bytes,
		/* 0x32 */ bytes,
		/* 0x33 */ bytes,
		/* 0x34 */ bytes,
		/* 0x35 */ bytes,
		/* 0x36 */ bytes,
		/* 0x37 */ bytes,
		/* 0x38 */ bytes,
		/* 0x39 */ bytes,
		/* 0x3a */ bytes,
		/* 0x3b */ bytes,
		/* 0x3c */ bytes,
		/* 0x3d */ bytes,
		/* 0x3e */ bytes,
		/* 0x3f */ bytes,
		/* 0x40 */ bytes,
		/* 0x41 */ bytes,
		/* 0x42 */ bytes,
		/* 0x43 */ bytes,
		/* 0x44 */ bytes,
		/* 0x45 */ bytes,
		/* 0x46 */ bytes,
		/* 0x47 */ bytes,
		/* 0x48 */ bytes,
		/* 0x49 */ bytes,
		/* 0x4a */ bytes,
		/* 0x4b */ bytes,
		/* 0x4c */ bytes,
		/* 0x4d */ bytes,
		/* 0x4e */ bytes,
		/* 0x4f */ bytes,
		/* 0x50 */ bytes,
		/* 0x51 */ bytes,
		/* 0x52 */ bytes,
		/* 0x53 */ bytes,
		/* 0x54 */ bytes,
		/* 0x55 */ bytes,
		/* 0x56 */ bytes,
		/* 0x57 */ bytes,
		/* 0x58 */ bytes,
		/* 0x59 */ bytes,
		/* 0x5a */ bytes,
		/* 0x5b */ bytes,
		/* 0x5c */ bytes,
		/* 0x5d */ bytes,
		/* 0x5e */ bytes,
		/* 0x5f */ bytes,
		/* 0x60 */ bytes,
		/* 0x61 */ bytes,
		/* 0x62 */ bytes,
		/* 0x63 */ bytes,
		/* 0x64 */ bytes,
		/* 0x65 */ bytes,
		/* 0x66 */ bytes,
		/* 0x67 */ bytes,
		/* 0x68 */ bytes,
		/* 0x69 */ bytes,
		/* 0x6a */ bytes,
		/* 0x6b */ bytes,
		/* 0x6c */ bytes,
		/* 0x6d */ bytes,
		/* 0x6e */ bytes,
		/* 0x6f */ bytes,
		/* 0x70 */ bytes,
		/* 0x71 */ bytes,
		/* 0x72 */ bytes,
		/* 0x73 */ bytes,
		/* 0x74 */ bytes,
		/* 0x75 */ bytes,
		/* 0x76 */ bytes,
		/* 0x77 */ bytes,
		/* 0x78 */ bytes,
		/* 0x79 */ bytes,
		/* 0x7a */ bytes,
		/* 0x7b */ bytes,
		/* 0x7c */ bytes,
		/* 0x7d */ bytes,
		/* 0x7e */ bytes,
		/* 0x7f */ bytes,
		/* 0x80 */ bytes,
		/* 0x81 */ bytes,
		/* 0x82 */ bytes,
		/* 0x83 */ bytes,
		/* 0x84 */ bytes,
		/* 0x85 */ bytes,
		/* 0x86 */ bytes,
		/* 0x87 */ bytes,
		/* 0x88 */ mov_rm_reg_8,
		/* 0x89 */ mov_rm_reg_16,
		/* 0x8a */ mov_reg_rm_8,
		/* 0x8b */ mov_reg_rm_16,
		/* 0x8c */ bytes,
		/* 0x8d */ bytes,
		/* 0x8e */ bytes,
		/* 0x8f */ bytes,
		/* 0x90 */ bytes,
		/* 0x91 */ bytes,
		/* 0x92 */ bytes,
		/* 0x93 */ bytes,
		/* 0x94 */ bytes,
		/* 0x95 */ bytes,
		/* 0x96 */ bytes,
		/* 0x97 */ bytes,
		/* 0x98 */ bytes,
		/* 0x99 */ bytes,
		/* 0x9a */ bytes,
		/* 0x9b */ bytes,
		/* 0x9c */ bytes,
		/* 0x9d */ bytes,
		/* 0x9e */ bytes,
		/* 0x9f */ bytes,
		/* 0xa0 */ bytes,
		/* 0xa1 */ mov_ax_mem,
		/* 0xa2 */ bytes,
		/* 0xa3 */ mov_mem_ax,
		/* 0xa4 */ bytes,
		/* 0xa5 */ bytes,
		/* 0xa6 */ bytes,
		/* 0xa7 */ bytes,
		/* 0xa8 */ bytes,
		/* 0xa9 */ bytes,
		/* 0xaa */ bytes,
		/* 0xab */ bytes,
		/* 0xac */ bytes,
		/* 0xad */ bytes,
		/* 0xae */ bytes,
		/* 0xaf */ bytes,
		/* 0xb0 */ bytes,
		/* 0xb1 */ mov_cl_immed,
		/* 0xb2 */ bytes,
		/* 0xb3 */ bytes,
		/* 0xb4 */ bytes,
		/* 0xb5 */ mov_ch_immed,
		/* 0xb6 */ bytes,
		/* 0xb7 */ bytes,
		/* 0xb8 */ bytes,
		/* 0xb9 */ mov_cx_immed,
		/* 0xba */ mov_dx_immed,
		/* 0xbb */ bytes,
		/* 0xbc */ bytes,
		/* 0xbd */ bytes,
		/* 0xbe */ bytes,
		/* 0xbf */ bytes,
		/* 0xc0 */ bytes,
		/* 0xc1 */ bytes,
		/* 0xc2 */ bytes,
		/* 0xc3 */ bytes,
		/* 0xc4 */ bytes,
		/* 0xc5 */ bytes,
		/* 0xc6 */ mov_mem_immed_8,
		/* 0xc7 */ mov_mem_immed_16,
		/* 0xc8 */ bytes,
		/* 0xc9 */ bytes,
		/* 0xca */ bytes,
		/* 0xcb */ bytes,
		/* 0xcc */ bytes,
		/* 0xcd */ bytes,
		/* 0xce */ bytes,
		/* 0xcf */ bytes,
		/* 0xd0 */ bytes,
		/* 0xd1 */ bytes,
		/* 0xd2 */ bytes,
		/* 0xd3 */ bytes,
		/* 0xd4 */ bytes,
		/* 0xd5 */ bytes,
		/* 0xd6 */ bytes,
		/* 0xd7 */ bytes,
		/* 0xd8 */ bytes,
		/* 0xd9 */ bytes,
		/* 0xda */ bytes,
		/* 0xdb */ bytes,
		/* 0xdc */ bytes,
		/* 0xdd */ bytes,
		/* 0xde */ bytes,
		/* 0xdf */ bytes,
		/* 0xe0 */ bytes,
		/* 0xe1 */ bytes,
		/* 0xe2 */ bytes,
		/* 0xe3 */ bytes,
		/* 0xe4 */ bytes,
		/* 0xe5 */ bytes,
		/* 0xe6 */ bytes,
		/* 0xe7 */ bytes,
		/* 0xe8 */ bytes,
		/* 0xe9 */ bytes,
		/* 0xea */ bytes,
		/* 0xeb */ bytes,
		/* 0xec */ bytes,
		/* 0xed */ bytes,
		/* 0xee */ bytes,
		/* 0xef */ bytes,
		/* 0xf0 */ bytes,
		/* 0xf1 */ bytes,
		/* 0xf2 */ bytes,
		/* 0xf3 */ bytes,
		/* 0xf4 */ bytes,
		/* 0xf5 */ bytes,
		/* 0xf6 */ bytes,
		/* 0xf7 */ bytes,
		/* 0xf8 */ bytes,
		/* 0xf9 */ bytes,
		/* 0xfa */ bytes,
		/* 0xfb */ bytes,
		/* 0xfc */ bytes,
		/* 0xfd */ bytes,
		/* 0xfe */ bytes,
		/* 0xff */ bytes,
	};
}

namespace sim86
{
	PrintResult print(const std::uint8_t* first, const std::uint8_t* last)
	{
		if (first == last) return { "", last };
		return s_printers[*first](first, last);
	}
}
