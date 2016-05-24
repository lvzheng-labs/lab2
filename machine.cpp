#include "machine.hpp"

#include "error.hpp"

namespace BASIC {

void machine::run(const binary_code_t& prog)
{
	reg.PC = reg.STEP = reg.STOP = 0;
	while (!reg.STOP && static_cast<size_t>(reg.PC) < prog.size()) {
		step(prog[reg.PC]);
	}
}

void machine::step(const instruction& ins)
{
	++reg.PC;
	++reg.STEP;
	auto op = ins.op_lo >> 4;
	auto mode = ins.op_lo & 0x0f;

	switch (op) {
	case instruction::OP_NOP:
		break;
	case instruction::OP_INT:
		assert(ins.operand[0] == 0xff);
		throw error::line_number_error();
	case instruction::OP_HALT:
		reg.STOP = 1;
		break;
	case instruction::OP_PRINT:
		print_number(stack.top());
		stack.pop();
		break;
	case instruction::OP_INPUT:
		stack.push(input_number());
		break;
	case instruction::OP_PUSH:
		if (mode == 0x01) {
			stack.push(ins.operand[0]);
		} else {
			if (!vars[ins.operand[0]])
				throw error::variable_not_defined();
			stack.push(*vars[ins.operand[0]]);
		}
		break;
	case instruction::OP_POP:
		vars[ins.operand[0]] = stack.top();
		stack.pop();
		break;
	case instruction::OP_ADD: {
		integer_t n = stack.top();
		stack.pop();
		stack.top() += n;
		break; }
	case instruction::OP_SUB: {
		integer_t n = stack.top();
		stack.pop();
		stack.top() -= n;
		break; }
	case instruction::OP_MUL: {
		integer_t n = stack.top();
		stack.pop();
		stack.top() *= n;
		break; }
	case instruction::OP_DIV: {
		integer_t n = stack.top();
		if (n == 0)
			throw error::divided_by_zero();
		stack.pop();
		stack.top() /= n;
		break; }
	case instruction::OP_JMP:
		reg.PC = ins.operand[0];
		break;
	case instruction::OP_JZ: {
		integer_t n = stack.top();
		stack.pop();
		if (n == 0)
			reg.PC = ins.operand[0];
		break; }
	case instruction::OP_JP: {
		integer_t n = stack.top();
		stack.pop();
		if (n > 0)
			reg.PC = ins.operand[0];
		break; }
	default:
		assert(0);
	}
	return;
}

void machine::clear()
{
	var_map.clear();
	vars.clear();
	stack = stack_t();
	reg.PC = reg.STEP = reg.STOP = 0;
}

} // namespace BASIC
