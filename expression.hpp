#ifndef BASIC_EXPRESSION_HPP
#define BASIC_EXPRESSION_HPP

#include "common.hpp"

namespace BASIC {

struct expr_token {
	enum {
		IMMEDIATE = 0,
		VARIABLE,
		OPERATOR,
		// These are for shunting-yard algo.
		LBRACE = 256,
		RBRACE = 257,
	} type;
	std::string str;
	integer_t num;
};

using expr_t = std::vector<expr_token>;

} // namespace BASIC

#endif // BASIC_EXPRESSION_HPP
