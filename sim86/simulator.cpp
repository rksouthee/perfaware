#include "simulator.h"

#include <iostream>

namespace
{
#define EXECUTE_FN(name) void name(const std::uint8_t* first, const std::uint8_t* last, sim86::Context& ctx)
	typedef EXECUTE_FN((*Execute_fn));

	EXECUTE_FN(noop)
	{
		std::cout << "skipping " << std::hex << static_cast<int>(*first) << std::endl;
		/* ctx.ip += 1; */
	}

	void store_little_endian(std::uint8_t* ptr, std::uint16_t val)
	{
		ptr[0] = val & 0xff;
		ptr[1] = (val >> 8) & 0xff;
	}

	void mov_reg_immed_16(std::size_t reg, const std::uint8_t* first, const std::uint8_t* last, sim86::Context& ctx)
	{
		auto* ptr = reinterpret_cast<std::uint8_t*>(&ctx.registers[reg]);
		const std::uint16_t immed = first[1] | (first[2] << 8);
		store_little_endian(ptr, immed);
	}

	EXECUTE_FN(sub_rm_reg_16)
	{
		const std::uint8_t r_m = (first[1] >> 0) & 0x7;
		const std::uint8_t reg = (first[1] >> 3) & 0x7;
		ctx.registers[r_m] -= ctx.registers[reg];
		if (ctx.registers[r_m] >> 15)
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags | sim86::Context::Flags_sign);
		}
		else
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags & ~sim86::Context::Flags_sign);
		}

		if (ctx.registers[r_m] == 0)
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags | sim86::Context::Flags_zero);
		}
		else
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags & ~sim86::Context::Flags_zero);
		}
	}

	EXECUTE_FN(cmp_rm_reg_16)
	{
		const std::uint8_t r_m = (first[1] >> 0) & 0x7;
		const std::uint8_t reg = (first[1] >> 3) & 0x7;
		const std::uint16_t result = ctx.registers[r_m] - ctx.registers[reg];
		if (result >> 15)
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags | sim86::Context::Flags_sign);
		}
		else
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags & ~sim86::Context::Flags_sign);
		}

		if (result == 0)
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags | sim86::Context::Flags_zero);
		}
		else
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags & ~sim86::Context::Flags_zero);
		}
	}

	EXECUTE_FN(jnz_short_label)
	{
		const std::int8_t offset = static_cast<std::int8_t>(first[1]);
		if (!(ctx.flags & sim86::Context::Flags_zero))
		{
			ctx.ip += offset;
		}
	}

	EXECUTE_FN(op_rm_imm_16)
	{
		const std::uint8_t op = (first[1] >> 3) & 0x7;
		const std::uint8_t r_m = first[1] & 0x7;
		const std::uint16_t imm = first[2] | (first[3] << 8);
		std::uint16_t result = 0;
		switch (op)
		{
		case 0:
			result = ctx.registers[r_m] += imm;
			break;
		case 5:
			result = ctx.registers[r_m] -= imm;
			break;
		case 7:
			result = ctx.registers[r_m] - imm;
			break;
		default:
			std::cout << "unhandled op " << static_cast<int>(op) << std::endl;
			break;
		}
		if (result >> 15)
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags | sim86::Context::Flags_sign);
		}
		else
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags & ~sim86::Context::Flags_sign);
		}
		if (result == 0)
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags | sim86::Context::Flags_zero);
		}
		else
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags & ~sim86::Context::Flags_zero);
		}
	}

	EXECUTE_FN(op_rm16_immed8)
	{
		const std::uint8_t op = (first[1] >> 3) & 0x7;
		const std::uint8_t r_m = first[1] & 0x7;
		const auto imm = static_cast<std::int16_t>(static_cast<std::int8_t>(first[2]));
		std::uint16_t result = 0;
		switch (op)
		{
		case 0:
			result = ctx.registers[r_m] += imm;
			break;
		case 5:
			result = ctx.registers[r_m] -= imm;
			break;
		case 7:
			result = ctx.registers[r_m] - imm;
			break;
		default:
			std::cerr << "unhandled op " << static_cast<int>(op) << std::endl;
			break;
		}
		if (result >> 15)
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags | sim86::Context::Flags_sign);
		}
		else
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags & ~sim86::Context::Flags_sign);
		}
		if (result == 0)
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags | sim86::Context::Flags_zero);
		}
		else
		{
			ctx.flags = (sim86::Context::Flags)(ctx.flags & ~sim86::Context::Flags_zero);
		}
	}

	EXECUTE_FN(mov_ax_immed_16)
	{
		mov_reg_immed_16(0, first, last, ctx);
	}

	EXECUTE_FN(mov_cx_immed_16)
	{
		mov_reg_immed_16(1, first, last, ctx);
	}

	EXECUTE_FN(mov_dx_immed_16)
	{
		mov_reg_immed_16(2, first, last, ctx);
	}

	EXECUTE_FN(mov_bx_immed_16)
	{
		mov_reg_immed_16(3, first, last, ctx);
	}

	EXECUTE_FN(mov_sp_immed_16)
	{
		mov_reg_immed_16(4, first, last, ctx);
	}

	EXECUTE_FN(mov_bp_immed_16)
	{
		mov_reg_immed_16(5, first, last, ctx);
	}

	EXECUTE_FN(mov_si_immed_16)
	{
		mov_reg_immed_16(6, first, last, ctx);
	}

	EXECUTE_FN(mov_di_immed_16)
	{
		mov_reg_immed_16(7, first, last, ctx);
	}

	EXECUTE_FN(mov_rm_reg_16)
	{
		const std::uint8_t r_m = (first[1] >> 0) & 0x7;
		const std::uint8_t reg = (first[1] >> 3) & 0x7;
		ctx.registers[r_m] = ctx.registers[reg];
	}

	const Execute_fn s_executors[256] =
	{
		/* 0x00 */ noop,
		/* 0x01 */ noop,
		/* 0x02 */ noop,
		/* 0x03 */ noop,
		/* 0x04 */ noop,
		/* 0x05 */ noop,
		/* 0x06 */ noop,
		/* 0x07 */ noop,
		/* 0x08 */ noop,
		/* 0x09 */ noop,
		/* 0x0a */ noop,
		/* 0x0b */ noop,
		/* 0x0c */ noop,
		/* 0x0d */ noop,
		/* 0x0e */ noop,
		/* 0x0f */ noop,
		/* 0x10 */ noop,
		/* 0x11 */ noop,
		/* 0x12 */ noop,
		/* 0x13 */ noop,
		/* 0x14 */ noop,
		/* 0x15 */ noop,
		/* 0x16 */ noop,
		/* 0x17 */ noop,
		/* 0x18 */ noop,
		/* 0x19 */ noop,
		/* 0x1a */ noop,
		/* 0x1b */ noop,
		/* 0x1c */ noop,
		/* 0x1d */ noop,
		/* 0x1e */ noop,
		/* 0x1f */ noop,
		/* 0x20 */ noop,
		/* 0x21 */ noop,
		/* 0x22 */ noop,
		/* 0x23 */ noop,
		/* 0x24 */ noop,
		/* 0x25 */ noop,
		/* 0x26 */ noop,
		/* 0x27 */ noop,
		/* 0x28 */ noop,
		/* 0x29 */ sub_rm_reg_16,
		/* 0x2a */ noop,
		/* 0x2b */ noop,
		/* 0x2c */ noop,
		/* 0x2d */ noop,
		/* 0x2e */ noop,
		/* 0x2f */ noop,
		/* 0x30 */ noop,
		/* 0x31 */ noop,
		/* 0x32 */ noop,
		/* 0x33 */ noop,
		/* 0x34 */ noop,
		/* 0x35 */ noop,
		/* 0x36 */ noop,
		/* 0x37 */ noop,
		/* 0x38 */ noop,
		/* 0x39 */ cmp_rm_reg_16,
		/* 0x3a */ noop,
		/* 0x3b */ noop,
		/* 0x3c */ noop,
		/* 0x3d */ noop,
		/* 0x3e */ noop,
		/* 0x3f */ noop,
		/* 0x40 */ noop,
		/* 0x41 */ noop,
		/* 0x42 */ noop,
		/* 0x43 */ noop,
		/* 0x44 */ noop,
		/* 0x45 */ noop,
		/* 0x46 */ noop,
		/* 0x47 */ noop,
		/* 0x48 */ noop,
		/* 0x49 */ noop,
		/* 0x4a */ noop,
		/* 0x4b */ noop,
		/* 0x4c */ noop,
		/* 0x4d */ noop,
		/* 0x4e */ noop,
		/* 0x4f */ noop,
		/* 0x50 */ noop,
		/* 0x51 */ noop,
		/* 0x52 */ noop,
		/* 0x53 */ noop,
		/* 0x54 */ noop,
		/* 0x55 */ noop,
		/* 0x56 */ noop,
		/* 0x57 */ noop,
		/* 0x58 */ noop,
		/* 0x59 */ noop,
		/* 0x5a */ noop,
		/* 0x5b */ noop,
		/* 0x5c */ noop,
		/* 0x5d */ noop,
		/* 0x5e */ noop,
		/* 0x5f */ noop,
		/* 0x60 */ noop,
		/* 0x61 */ noop,
		/* 0x62 */ noop,
		/* 0x63 */ noop,
		/* 0x64 */ noop,
		/* 0x65 */ noop,
		/* 0x66 */ noop,
		/* 0x67 */ noop,
		/* 0x68 */ noop,
		/* 0x69 */ noop,
		/* 0x6a */ noop,
		/* 0x6b */ noop,
		/* 0x6c */ noop,
		/* 0x6d */ noop,
		/* 0x6e */ noop,
		/* 0x6f */ noop,
		/* 0x70 */ noop,
		/* 0x71 */ noop,
		/* 0x72 */ noop,
		/* 0x73 */ noop,
		/* 0x74 */ noop,
		/* 0x75 */ jnz_short_label,
		/* 0x76 */ noop,
		/* 0x77 */ noop,
		/* 0x78 */ noop,
		/* 0x79 */ noop,
		/* 0x7a */ noop,
		/* 0x7b */ noop,
		/* 0x7c */ noop,
		/* 0x7d */ noop,
		/* 0x7e */ noop,
		/* 0x7f */ noop,
		/* 0x80 */ noop,
		/* 0x81 */ op_rm_imm_16,
		/* 0x82 */ noop,
		/* 0x83 */ op_rm16_immed8,
		/* 0x84 */ noop,
		/* 0x85 */ noop,
		/* 0x86 */ noop,
		/* 0x87 */ noop,
		/* 0x88 */ noop,
		/* 0x89 */ mov_rm_reg_16,
		/* 0x8a */ noop,
		/* 0x8b */ noop,
		/* 0x8c */ noop,
		/* 0x8d */ noop,
		/* 0x8e */ noop,
		/* 0x8f */ noop,
		/* 0x90 */ noop,
		/* 0x91 */ noop,
		/* 0x92 */ noop,
		/* 0x93 */ noop,
		/* 0x94 */ noop,
		/* 0x95 */ noop,
		/* 0x96 */ noop,
		/* 0x97 */ noop,
		/* 0x98 */ noop,
		/* 0x99 */ noop,
		/* 0x9a */ noop,
		/* 0x9b */ noop,
		/* 0x9c */ noop,
		/* 0x9d */ noop,
		/* 0x9e */ noop,
		/* 0x9f */ noop,
		/* 0xa0 */ noop,
		/* 0xa1 */ noop,
		/* 0xa2 */ noop,
		/* 0xa3 */ noop,
		/* 0xa4 */ noop,
		/* 0xa5 */ noop,
		/* 0xa6 */ noop,
		/* 0xa7 */ noop,
		/* 0xa8 */ noop,
		/* 0xa9 */ noop,
		/* 0xaa */ noop,
		/* 0xab */ noop,
		/* 0xac */ noop,
		/* 0xad */ noop,
		/* 0xae */ noop,
		/* 0xaf */ noop,
		/* 0xb0 */ noop,
		/* 0xb1 */ noop,
		/* 0xb2 */ noop,
		/* 0xb3 */ noop,
		/* 0xb4 */ noop,
		/* 0xb5 */ noop,
		/* 0xb6 */ noop,
		/* 0xb7 */ noop,
		/* 0xb8 */ mov_ax_immed_16,
		/* 0xb9 */ mov_cx_immed_16,
		/* 0xba */ mov_dx_immed_16,
		/* 0xbb */ mov_bx_immed_16,
		/* 0xbc */ mov_sp_immed_16,
		/* 0xbd */ mov_bp_immed_16,
		/* 0xbe */ mov_si_immed_16,
		/* 0xbf */ mov_di_immed_16,
		/* 0xc0 */ noop,
		/* 0xc1 */ noop,
		/* 0xc2 */ noop,
		/* 0xc3 */ noop,
		/* 0xc4 */ noop,
		/* 0xc5 */ noop,
		/* 0xc6 */ noop,
		/* 0xc7 */ noop,
		/* 0xc8 */ noop,
		/* 0xc9 */ noop,
		/* 0xca */ noop,
		/* 0xcb */ noop,
		/* 0xcc */ noop,
		/* 0xcd */ noop,
		/* 0xce */ noop,
		/* 0xcf */ noop,
		/* 0xd0 */ noop,
		/* 0xd1 */ noop,
		/* 0xd2 */ noop,
		/* 0xd3 */ noop,
		/* 0xd4 */ noop,
		/* 0xd5 */ noop,
		/* 0xd6 */ noop,
		/* 0xd7 */ noop,
		/* 0xd8 */ noop,
		/* 0xd9 */ noop,
		/* 0xda */ noop,
		/* 0xdb */ noop,
		/* 0xdc */ noop,
		/* 0xdd */ noop,
		/* 0xde */ noop,
		/* 0xdf */ noop,
		/* 0xe0 */ noop,
		/* 0xe1 */ noop,
		/* 0xe2 */ noop,
		/* 0xe3 */ noop,
		/* 0xe4 */ noop,
		/* 0xe5 */ noop,
		/* 0xe6 */ noop,
		/* 0xe7 */ noop,
		/* 0xe8 */ noop,
		/* 0xe9 */ noop,
		/* 0xea */ noop,
		/* 0xeb */ noop,
		/* 0xec */ noop,
		/* 0xed */ noop,
		/* 0xee */ noop,
		/* 0xef */ noop,
		/* 0xf0 */ noop,
		/* 0xf1 */ noop,
		/* 0xf2 */ noop,
		/* 0xf3 */ noop,
		/* 0xf4 */ noop,
		/* 0xf5 */ noop,
		/* 0xf6 */ noop,
		/* 0xf7 */ noop,
		/* 0xf8 */ noop,
		/* 0xf9 */ noop,
		/* 0xfa */ noop,
		/* 0xfb */ noop,
		/* 0xfc */ noop,
		/* 0xfd */ noop,
		/* 0xfe */ noop,
		/* 0xff */ noop,
	};
}

namespace sim86
{
	void execute(const std::uint8_t* first, const std::uint8_t* last, Context& ctx)
	{
		if (first == last) return;
		s_executors[*first](first, last, ctx);
	}
}
