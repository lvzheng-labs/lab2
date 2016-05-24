#ifndef BASIC_INSTRUCTION_HPP
#define BASIC_INSTRUCTION_HPP

#include "common.hpp"

namespace BASIC {

struct instruction {
	enum op_type {
		OP_NOP		= 0,
		OP_INT,
		OP_HALT,
		OP_PRINT,
		OP_INPUT,
		OP_PUSH		= 5,
		OP_POP,
		OP_ADD,
		OP_SUB,
		OP_MUL,
		OP_DIV		= 10,
		OP_JMP,
		OP_JZ,
		OP_JP,
	};
	union {
		struct {
			short_t op_lo;
			short_t op_hi;
		};
		integer_t op;
	};
	enum {
		NOPERANDS = 3,
	};
	integer_t operand[NOPERANDS];
};

using binary_code_t = std::vector<instruction>;

constexpr std::size_t INSTRUCTION_ASM_MAXLEN = 8;
const char asm_lang[][INSTRUCTION_ASM_MAXLEN] = {
	"NOP",
	"INT",
	"HALT",
	"PRINT",
	"INPUT",
	"PUSH",
	"POP",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"JMP",
	"JZ",
	"JP",
};

} // namespace BASIC

#endif // BASIC_INSTRUCTION_HPP
