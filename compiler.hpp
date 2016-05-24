#ifndef BASIC_COMPILER_HPP
#define BASIC_COMPILER_HPP

#include "common.hpp"

#include "command.hpp"

namespace BASIC {

class compiler {
public:
	command compile(const std::string& basic);

private:
	// command under parse
	command comm;
	// string stream for parsing
	std::istringstream ss;

	// Store a reserved word instead if it was consumed as a variable.
	std_optional<std::string> stored_keyword;

	// Throws on error.
	void var_target();
	void let_equal();
	void shunting_yard_expr();
	void lineno_target();
	void if_condition();
	void if_then();

	// Return nullopt on error.
	std_optional<integer_t> consume_num();
	std_optional<std::string> consume_var();
	std_optional<expr_token> consume_value();
	std_optional<expr_token> consume_token();

	// Get comparison operator. Throws on error.
	expr_token get_comp();

	// Ignore all blank characters. Return false if EOF is reached.
	bool pass_blank(); 

	// Return whether precedence of lhs <= that of rhs.
	bool precedence_le(const expr_token& lhs, const expr_token& rhs);

	// Check whether the command end without trailing extra stuffs.
	void command_end();
};

} // namespace BASIC

#endif // BASIC_COMPILER_HPP
