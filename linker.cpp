#include "linker.hpp"

#include "error.hpp"

namespace BASIC {

binary_code_t linker::link(const object_code_t& obj)
{
	bin.clear();
	l2l.clear();
	lineno_map.clear();

	for (auto& line : obj) {
		lineno_map[line.first] = bin.size();
		auto& a = line.second;
		switch (a.type) {
		case command::BASIC_REM:
			break;
		case command::BASIC_LET:
			expand_expr(a.expr);
			pop_to_var(a.target_var);
			break;
		case command::BASIC_PRINT:
			expand_expr(a.expr);
			program_print();
			break;
		case command::BASIC_INPUT:
			input_variable(a.target_var);
			break;
		case command::BASIC_GOTO:
			program_goto(a.target_lineno);
			break;
		case command::BASIC_IF:
			if_condition(a.expr, a.expr2, a.cmp, a.target_lineno);
			break;
		case command::BASIC_END:
			program_end();
			break;
		default:
			assert(0);
		}
	}

	linkall_lineno();

	return bin;
}

void linker::expand_expr(const expr_t& expr)
{
	for (auto& token : expr) {
		instruction ins;
		std::memset(&ins, 0, sizeof(ins));
		switch (token.type) {
		case expr_token::IMMEDIATE:
			ins.op_lo = (instruction::OP_PUSH << 4) | 1;
			ins.operand[0] = token.num;
			break;
		case expr_token::VARIABLE:
			ins.op_lo = (instruction::OP_PUSH << 4) | 2;
			ins.operand[0] = get_var_addr(token.str);
			break;
		case expr_token::OPERATOR:
			ins.op_lo = (get_operator_op(token.str) << 4) | 0;
			break;
		default:
			assert(0);
		}
		bin.push_back(std::move(ins));
	}
}

void linker::pop_to_var(const std::string& var)
{
	auto addr = get_var_addr(var);
	instruction ins;
	std::memset(&ins, 0, sizeof(ins));
	ins.op_lo = (instruction::OP_POP << 4) | 2;
	ins.operand[0] = addr;
	bin.push_back(std::move(ins));
}

void linker::program_print()
{
	instruction ins;
	std::memset(&ins, 0, sizeof(ins));
	ins.op_lo = (instruction::OP_PRINT << 4) | 0;
	bin.push_back(std::move(ins));
}

void linker::input_variable(const std::string& var)
{
	instruction ins;
	std::memset(&ins, 0, sizeof(ins));
	ins.op_lo = (instruction::OP_INPUT << 4) | 0;
	bin.push_back(std::move(ins));
	pop_to_var(var);
}

void linker::program_goto(std::size_t lineno)
{
	instruction ins;
	std::memset(&ins, 0, sizeof(ins));
	ins.op_lo = (instruction::OP_JMP << 4) | 8;
	ask_lineno(lineno);
	bin.push_back(std::move(ins));
}

void linker::if_condition(const expr_t& exprl, const expr_t& exprr,
		const std::string& cmp, std::size_t lineno)
{
	auto do_sub = [this]() {
		instruction ins;
		std::memset(&ins, 0, sizeof(ins));
		ins.op_lo = (instruction::OP_SUB << 4 | 0);
		bin.push_back(std::move(ins));
	};
	instruction ins;
	std::memset(&ins, 0, sizeof(ins));
	if (cmp == "=") {
		expand_expr(exprl);
		expand_expr(exprr);
		do_sub();
		ins.op_lo = (instruction::OP_JZ << 4) | 8;
	} else if (cmp == ">") {
		expand_expr(exprl);
		expand_expr(exprr);
		do_sub();
		ins.op_lo = (instruction::OP_JP << 4) | 8;
	} else if (cmp == "<") {
		expand_expr(exprr);
		expand_expr(exprl);
		do_sub();
		ins.op_lo = (instruction::OP_JP << 4) | 8;
	} else {
		assert(0);
	}
	ask_lineno(lineno);
	bin.push_back(std::move(ins));
}

void linker::program_end()
{
	instruction ins;
	std::memset(&ins, 0, sizeof(ins));
	ins.op_lo = (instruction::OP_HALT << 4) | 0;
	bin.push_back(std::move(ins));
}

void linker::push_number(const expr_token& token)
{
	instruction ins;
	std::memset(&ins, 0, sizeof(ins));
	if (token.type == expr_token::IMMEDIATE) {
		ins.op_lo = (instruction::OP_PUSH << 4) | 1;
		ins.operand[0] = token.num;
	} else if (token.type == expr_token::VARIABLE) {
		ins.op_lo = (instruction::OP_PUSH << 4) | 2;
		ins.operand[0] = get_var_addr(token.str);
	}
	bin.push_back(std::move(ins));
}

integer_t linker::get_var_addr(const std::string& var)
{
	auto it = _mach.var_map.find(var);
	if (it == _mach.var_map.end()) {
		integer_t result = _mach.vars.size();
		_mach.var_map.emplace(var, result);
		_mach.vars.push_back(std_nullopt);
		return result;
	} else {
		return it->second;
	}
}

short_t linker::get_operator_op(const std::string& oper)
{
	short_t result;
	switch (oper[0]) {
	case '+':
		result = instruction::OP_ADD;
		break;
	case '-':
		result = instruction::OP_SUB;
		break;
	case '*':
		result = instruction::OP_MUL;
		break;
	case '/':
		result = instruction::OP_DIV;
		break;
	default:
		assert(0);
	}
	return result;
}

void linker::ask_lineno(std::size_t lineno)
{
	l2l.push_back({bin.size(), 0, lineno});
}

void linker::linkall_lineno()
{
	for (auto& entry : l2l) {
		auto it = lineno_map.find(entry.lineno);
		auto& ins = bin[entry.id_bin];
		if (it == lineno_map.end()) {
			std::memset(&ins, 0, sizeof(ins));
			ins.op_lo = (instruction::OP_INT << 4) | 1;
			ins.operand[0] = 0xff;
		} else {
			ins.operand[entry.id_operand] = it->second;
		}
	}
}

void print_program(std::ostream& os, binary_code_t& prog)
{
	std::size_t l = 0;
	for (auto& ins : prog) {
		os << l++ << '\t';
		os << asm_lang[ins.op_lo >> 4];
		auto type = ins.op_lo & 0x0f;
		if (type != 0) {
			os << '\t';
			switch (type) {
			case 1: // IMMEDIATE
				os << "%";
				break;
			case 2: // VARIABLE
				os << "$";
				break;
			case 8: // LINENO
				os << "#";
				break;
			default:
				assert(0);
			}
			os << ins.operand[0];
		}
		os << std::endl;
	}
}

} // namespace BASIC
