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

	const char* const s_ops[8] =
	{
		"add", "or", "adc", "sbb", "and", "sub", "xor", "cmp"
	};

	const char* get_op(const std::size_t op)
	{
		return s_ops[op];
	}

	std::optional<sim86::PrintResult> print_mod_rm(const std::uint8_t* first, const std::uint8_t* last, const std::uint8_t mod, const std::uint8_t r_m, const std::uint8_t w, const bool add_prefix = false)
	{
		const std::string prefix = add_prefix ? (w ? " word " : " byte ") : "";
		switch (mod)
		{
		case 0b00:
			{
				if (r_m == 0b110)
				{
					if (last - first < 2) return std::nullopt;
					const std::uint16_t addr = first[0] | (first[1] << 8);
					return sim86::PrintResult{ std::format("{}[{:#x}]", prefix, addr), first + 2 };
				}
				return sim86::PrintResult{ std::format("{}[{}]", prefix, s_ea_registers[r_m]), first };
			}
		case 0b01:
			{
				if (last - first < 1) return std::nullopt;
				return sim86::PrintResult{
					std::format("{}[{}{:+#x}]", prefix, s_ea_registers[r_m], static_cast<int>(static_cast<std::int8_t>(first[0]))),
					first + 1
				};
			}
		case 0b10:
			{
				if (last - first < 2) return std::nullopt;
				const std::int16_t addr = first[0] | (first[1] << 8);
				return sim86::PrintResult{
					std::format("{}[{}{:+#x}]", prefix, s_ea_registers[r_m], addr),
					first + 2
				};
			}
		case 0b11:
			{
				return sim86::PrintResult{ get_register(r_m, w), first };
			}
		}
		return std::nullopt;
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

		const std::optional<sim86::PrintResult> print_r_m = print_mod_rm(first, last, mod, r_m, w);
		if (!print_r_m) return std::nullopt;
		Mod_reg_rm result;
		result.r_m = print_r_m->code;
		result.reg = get_register(reg, w);
		result.end = print_r_m->end;
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

	sim86::PrintResult op_rm_reg(const char* op, const std::uint8_t* first, const std::uint8_t* last, const bool w, const bool d)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::optional<Mod_reg_rm> result = parse_mod_reg_rm(first, last, w);
		if (!result) return bytes(first - 1, last);
		std::ostringstream oss;
		oss << op << ' ';
		if (d) oss << result->reg << ',' << result->r_m;
		else oss << result->r_m << ',' << result->reg;
		return { oss.str(), result->end };
	}

	sim86::PrintResult op_reg_immed_8(const char* op, const std::uint8_t* first, const std::uint8_t* last, const char* const reg)
	{
		if (last - first < 2) return bytes(first, last);
		return { std::format("{} {},{:#x}", op, reg, static_cast<int>(first[1])), first + 2 };
	}

	sim86::PrintResult op_reg_immed_16(const char* op, const std::uint8_t* first, const std::uint8_t* last, const char* const reg)
	{
		if (last - first < 3) return bytes(first, last);
		const std::uint16_t immed = first[1] | (first[2] << 8);
		return { std::format("{} {},{:#x}", op, reg, immed), first + 3 };
	}

	sim86::PrintResult op_mem_immed_8(const char* op, const std::uint8_t* first, const std::uint8_t* last)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::optional<Mod_reg_rm> result = parse_mod_reg_rm(first, last, 0);
		if (!result || last - result->end < 1) return bytes(first - 1, last);
		return { std::format("{} byte {},{:#x}", op, result->r_m, static_cast<int>(result->end[0])), result->end + 1 };
	}

	sim86::PrintResult op_mem_immed_16(const char* op, const std::uint8_t* first, const std::uint8_t* last)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::optional<Mod_reg_rm> result = parse_mod_reg_rm(first, last, 1);
		if (!result || last - result->end < 2) return bytes(first - 1, last);
		const std::uint16_t immed = result->end[0] | (result->end[1] << 8);
		return { std::format("{} word {},{:#x}", op, result->r_m, immed), result->end + 2 };
	}

	sim86::PrintResult op_ax_mem(const char* op, const std::uint8_t* first, const std::uint8_t* last)
	{
		if (last - first < 3) return bytes(first, last);
		const std::uint16_t addr = first[1] | (first[2] << 8);
		return { std::format("{} ax,[{:#x}]", op, addr), first + 3 };
	}

	sim86::PrintResult op_mem_ax(const char* op, const std::uint8_t* first, const std::uint8_t* last)
	{
		if (last - first < 3) return bytes(first, last);
		const std::uint16_t addr = first[1] | (first[2] << 8);
		return { std::format("{} [{:#x}],ax", op, addr), first + 3 };
	}

	PRINT_FN(mov_cl_immed)
	{
		return op_reg_immed_8("mov", first, last, "cl");
	}

	PRINT_FN(mov_ch_immed)
	{
		return op_reg_immed_8("mov", first, last, "ch");
	}

	PRINT_FN(mov_cx_immed)
	{
		return op_reg_immed_16("mov", first, last, "cx");
	}

	PRINT_FN(mov_dx_immed)
	{
		return op_reg_immed_16("mov", first, last, "dx");
	}

	PRINT_FN(mov_bx_immed)
	{
		return op_reg_immed_16("mov", first, last, "bx");
	}

	PRINT_FN(mov_sp_immed)
	{
		return op_reg_immed_16("mov", first, last, "sp");
	}

	PRINT_FN(mov_bp_immed)
	{
		return op_reg_immed_16("mov", first, last, "bp");
	}

	PRINT_FN(mov_si_immed)
	{
		return op_reg_immed_16("mov", first, last, "si");
	}

	PRINT_FN(mov_di_immed)
	{
		return op_reg_immed_16("mov", first, last, "di");
	}

	PRINT_FN(mov_mem_immed_8)
	{
		return op_mem_immed_8("mov", first, last);
	}

	PRINT_FN(mov_mem_immed_16)
	{
		return op_mem_immed_16("mov", first, last);
	}

	PRINT_FN(mov_ax_mem)
	{
		return op_ax_mem("mov", first, last);
	}

	PRINT_FN(mov_mem_ax)
	{
		return op_mem_ax("mov", first, last);
	}

	PRINT_FN(op_rm_8_immed_8)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::uint8_t value = *first++;
		const std::uint8_t r_m = value & 0x7;
		const std::uint8_t op = (value >> 3) & 0x7;
		const std::uint8_t mod = (value >> 6) & 0x3;

		const std::optional<sim86::PrintResult> print_rm = print_mod_rm(first, last, mod, r_m, 0);
		if (!print_rm) return bytes(first - 2, last);
		if (last - print_rm->end < 1) return bytes(first - 2, last); 
		const std::uint8_t immed = print_rm->end[0];
		return { std::format("{} {},byte {:#x}", get_op(op), print_rm->code, immed), print_rm->end + 1 };
	}

	PRINT_FN(op_rm_16_immed_8)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::uint8_t value = *first++;
		const std::uint8_t r_m = value & 0x7;
		const std::uint8_t op = (value >> 3) & 0x7;
		const std::uint8_t mod = (value >> 6) & 0x3;

		const std::optional<sim86::PrintResult> print_rm = print_mod_rm(first, last, mod, r_m, 1, 1);
		if (!print_rm) return bytes(first - 2, last);
		if (last - print_rm->end < 1) return bytes(first - 2, last); 
		const std::uint8_t immed = print_rm->end[0];
		return { std::format("{} {},byte {:+#x}", get_op(op), print_rm->code, immed), print_rm->end + 1 };
	}

	PRINT_FN(op_rm_16_immed_16)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::uint8_t value = *first++;
		const std::uint8_t r_m = value & 0x7;
		const std::uint8_t op = (value >> 3) & 0x7;
		const std::uint8_t mod = (value >> 6) & 0x3;

		const std::optional<sim86::PrintResult> print_rm = print_mod_rm(first, last, mod, r_m, 1);
		if (!print_rm) return bytes(first - 2, last);
		if (last - print_rm->end < 2) return bytes(first - 2, last); 
		const std::uint16_t immed = print_rm->end[0] | (print_rm->end[1] << 8);
		return { std::format("{} {},byte {:#x}", get_op(op), print_rm->code, immed), print_rm->end + 2 };
	}

	sim86::PrintResult print_jump(const char* jmp, const std::uint8_t* first, const std::uint8_t* last)
	{
		if (++first == last) return bytes(first - 1, last);
		const std::int8_t offset = first[0];
		/* return { std::format("{} short {:+#x}", jmp, offset), first + 1 }; */
		return { std::format("{} ${:+#x}", jmp, offset + 2), first + 1 };
	}

#define JUMP(jmp)\
	PRINT_FN(jmp##_short_label)\
	{\
		return print_jump(#jmp, first, last);\
	}

	JUMP(jo);
	JUMP(jno);
	JUMP(jc);
	JUMP(jnc);
	JUMP(jz);
	JUMP(jnz);
	JUMP(jna);
	JUMP(ja);
	JUMP(js);
	JUMP(jns);
	JUMP(jpe);
	JUMP(jpo);
	JUMP(jl);
	JUMP(jnl);
	JUMP(jng);
	JUMP(jg);
	JUMP(loope);
	JUMP(loopne);
	JUMP(loop);
	JUMP(jcxz);

#define BINARY_OPS(op)\
	PRINT_FN(op##_rm_reg_8)\
	{\
		return op_rm_reg(#op, first, last, 0, 0);\
	}\
	PRINT_FN(op##_rm_reg_16)\
	{\
		return op_rm_reg(#op, first, last, 1, 0);\
	}\
	PRINT_FN(op##_reg_rm_8)\
	{\
		return op_rm_reg(#op, first, last, 0, 1);\
	}\
	PRINT_FN(op##_reg_rm_16)\
	{\
		return op_rm_reg(#op, first, last, 1, 1);\
	}\
	PRINT_FN(op##_al_immed)\
	{\
		return op_reg_immed_8(#op, first, last, "al");\
	}\
	PRINT_FN(op##_ax_immed)\
	{\
		return op_reg_immed_16(#op, first, last, "ax");\
	}

	BINARY_OPS(add);
	BINARY_OPS(sub);
	BINARY_OPS(cmp);
	BINARY_OPS(mov);

	const Print_fn s_printers[256] =
	{
		/* 0x00 */ add_rm_reg_8,
		/* 0x01 */ add_rm_reg_16,
		/* 0x02 */ add_reg_rm_8,
		/* 0x03 */ add_reg_rm_16,
		/* 0x04 */ add_al_immed,
		/* 0x05 */ add_ax_immed,
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
		/* 0x28 */ sub_rm_reg_8,
		/* 0x29 */ sub_rm_reg_16,
		/* 0x2a */ sub_reg_rm_8,
		/* 0x2b */ sub_reg_rm_16,
		/* 0x2c */ sub_al_immed,
		/* 0x2d */ sub_ax_immed,
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
		/* 0x38 */ cmp_rm_reg_8,
		/* 0x39 */ cmp_rm_reg_16,
		/* 0x3a */ cmp_reg_rm_8,
		/* 0x3b */ cmp_reg_rm_16,
		/* 0x3c */ cmp_al_immed,
		/* 0x3d */ cmp_ax_immed,
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
		/* 0x70 */ jo_short_label,
		/* 0x71 */ jno_short_label,
		/* 0x72 */ jc_short_label,
		/* 0x73 */ jnc_short_label,
		/* 0x74 */ jz_short_label,
		/* 0x75 */ jnz_short_label,
		/* 0x76 */ jna_short_label,
		/* 0x77 */ ja_short_label,
		/* 0x78 */ js_short_label,
		/* 0x79 */ jns_short_label,
		/* 0x7a */ jpe_short_label,
		/* 0x7b */ jpo_short_label,
		/* 0x7c */ jl_short_label,
		/* 0x7d */ jnl_short_label,
		/* 0x7e */ jng_short_label,
		/* 0x7f */ jg_short_label,
		/* 0x80 */ op_rm_8_immed_8,
		/* 0x81 */ op_rm_16_immed_16,
		/* 0x82 */ bytes,
		/* 0x83 */ op_rm_16_immed_8,
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
		/* 0xb0 */ mov_al_immed,
		/* 0xb1 */ mov_cl_immed,
		/* 0xb2 */ bytes,
		/* 0xb3 */ bytes,
		/* 0xb4 */ bytes,
		/* 0xb5 */ mov_ch_immed,
		/* 0xb6 */ bytes,
		/* 0xb7 */ bytes,
		/* 0xb8 */ mov_ax_immed,
		/* 0xb9 */ mov_cx_immed,
		/* 0xba */ mov_dx_immed,
		/* 0xbb */ mov_bx_immed,
		/* 0xbc */ mov_sp_immed,
		/* 0xbd */ mov_bp_immed,
		/* 0xbe */ mov_si_immed,
		/* 0xbf */ mov_di_immed,
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
		/* 0xe0 */ loopne_short_label,
		/* 0xe1 */ loope_short_label,
		/* 0xe2 */ loop_short_label,
		/* 0xe3 */ jcxz_short_label,
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
