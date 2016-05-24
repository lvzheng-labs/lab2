#ifndef BASIC_COMMAND_HPP
#define BASIC_COMMAND_HPP

#include "common.hpp"

#include "expression.hpp"

namespace BASIC {

struct command {
	enum {
		BASIC_REM,
		BASIC_LET,
		BASIC_PRINT,
		BASIC_INPUT,
		BASIC_GOTO,
		BASIC_IF,
		BASIC_END,
	} type;
	expr_t expr;
	expr_t expr2; // second expr of IF statement
	std::string cmp; // comparation operator of IF statement
	std::size_t target_lineno;
	std::string target_var;
	void clear()
	{
		*this = command();
	}
};

using object_code_t = std::map<std::size_t, command>;

} // namespace BASIC

#endif // BASIC_COMMAND_HPP
