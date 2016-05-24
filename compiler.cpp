#include "compiler.hpp"

#include "error.hpp"

namespace BASIC {

command compiler::compile(const std::string& basic)
{
	comm.clear();
	stored_keyword = std_nullopt;
	// Clear the string stream. A bit complicated.
	ss.clear();
	ss.seekg(0);
	ss.str(basic);
	std::string comm_str;
	ss >> comm_str;
	if (!ss)
		throw error::empty_command();

	if (comm_str == "END") {
		comm.type = command::BASIC_END;
	} else if (comm_str == "GOTO") {
		comm.type = command::BASIC_GOTO;
		lineno_target();
	} else if (comm_str == "IF") {
		comm.type = command::BASIC_IF;
		if_condition();
		if_then();
		lineno_target();
	} else if (comm_str == "INPUT") {
		comm.type = command::BASIC_INPUT;
		var_target();
	} else if (comm_str == "LET") {
		comm.type = command::BASIC_LET;
		var_target();
		let_equal();
		shunting_yard_expr();
	} else if (comm_str == "PRINT") {
		comm.type = command::BASIC_PRINT;
		shunting_yard_expr();
	} else if (comm_str == "REM") {
		comm.type = command::BASIC_REM;
	}

	command_end();
	return comm;
}

void compiler::var_target()
{
	auto var = consume_var();
	if (!var)
		throw error::syntax_error();
	comm.target_var = std::move(*var);
}

void compiler::let_equal()
{
	char ch;
	ss >> ch;
	if (!ss || ch != '=')
		throw error::syntax_error();
}

void compiler::shunting_yard_expr()
{
	enum {
		TOKEN_NULL,
		TOKEN_VALUE,
		TOKEN_OPER,
	} prev = TOKEN_NULL;
	std::vector<expr_token> infix;
	while (1) {
		auto token = consume_token();
		if (!token)
			break;
		infix.push_back(std::move(*token));
	}
	expr_t& expr = comm.expr;
	std::stack<expr_token> operstack;
	for (auto& token : infix) {{
		if (token.type == expr_token::IMMEDIATE ||
				token.type == expr_token::VARIABLE) {
			if (prev == TOKEN_VALUE)
				throw error::syntax_error();
			prev = TOKEN_VALUE;
			expr.push_back(std::move(token));
			continue;
		}
		if (token.type == expr_token::OPERATOR) {
			if (prev != TOKEN_VALUE)
				throw error::syntax_error();
			prev = TOKEN_OPER;
			while (!operstack.empty() && operstack.top().type ==
					expr_token::OPERATOR &&
					precedence_le(token, operstack.top())) {
				expr.push_back(std::move(operstack.top()));
				operstack.pop();
			}
			operstack.push(std::move(token));
			continue;
		}
		if (token.type == expr_token::LBRACE) {
			operstack.push(std::move(token));
			continue;
		}
		if (token.type == expr_token::RBRACE) {
			while (1) {
				if (operstack.empty())
					throw error::syntax_error();
				auto next = std::move(operstack.top());
				operstack.pop();
				if (next.type == expr_token::LBRACE)
					break;
				expr.push_back(std::move(next));
			}
			continue;
		}
	}}
	if (prev != TOKEN_VALUE)
		throw error::syntax_error();
	while (!operstack.empty()) {
		auto& next = operstack.top();
		if (next.type == expr_token::LBRACE)
			throw error::syntax_error();
		expr.push_back(std::move(next));
		operstack.pop();
	}
}

void compiler::lineno_target()
{
	auto num = consume_num();
	if (!num)
		throw error::syntax_error();
	comm.target_lineno = *num;
}

void compiler::if_condition()
{
	shunting_yard_expr();
	std::swap(comm.expr, comm.expr2);
	comm.cmp = get_comp().str;
	shunting_yard_expr();
	std::swap(comm.expr, comm.expr2);
}

void compiler::if_then()
{
	if (!stored_keyword || *stored_keyword != "THEN")
		throw error::syntax_error();
	stored_keyword = std_nullopt;
}

std_optional<integer_t> compiler::consume_num()
{
	bool bl = pass_blank();
	if (!bl)
		return std_nullopt;

	std::string numstr;
	while (1) {
		int ch = ss.peek();
		if (ch == std::istringstream::traits_type::eof() ||
				!std::isdigit(ch))
			break;
		numstr.push_back(ch);
		ss.get();
	}
	if (numstr.empty())
		return std_nullopt;
	
	integer_t num;
	try {
		long long cnum = stoll(numstr);
		num = cnum > BASIC_INTEGER_MAX ? BASIC_INTEGER_MAX : cnum;
	} catch (std::invalid_argument&) {
		return std_nullopt;
	} catch (std::out_of_range&){
		num = BASIC_INTEGER_MAX;
	}

	return num;
}

std_optional<std::string> compiler::consume_var()
{
	bool bl = pass_blank();
	if (!bl)
		return std_nullopt;

	std::string varname;
	int ch = ss.peek();
	if (!std::isalpha(ch))
		return std_nullopt;
	ss.get();
	varname.push_back(ch);
	while (1) {
		ch = ss.peek();
		if (ch == std::istringstream::traits_type::eof() ||
				!std::isalnum(ch))
			break;
		varname.push_back(ch);
		ss.get();
	}
	if (reserved_words.count(varname)) {
		stored_keyword = varname;
		return std_nullopt;
	}
	return varname;
}

std_optional<expr_token> compiler::consume_value()
{
	expr_token v;
	auto num = consume_num();
	if (num) {
		v.type = expr_token::IMMEDIATE;
		v.num = *num;
		return v;
	}
	auto var = consume_var();
	if (var) {
		v.type = expr_token::VARIABLE;
		v.str = *var;
		return v;
	}
	return std_nullopt;
}

std_optional<expr_token> compiler::consume_token()
{
	bool bl = pass_blank();
	if (!bl)
		return std_nullopt;

	int ch = ss.get();
	if (ch == std::istringstream::traits_type::eof())
		return std_nullopt;

	expr_token token;
	if (ch == '(') {
		token.type = expr_token::LBRACE;
		return token;
	} else if (ch == ')') {
		token.type = expr_token::RBRACE;
		return token;
	} else if (ch == '+' || ch == '-' ||
			ch == '*' || ch == '/'
			) {
		token.type = expr_token::OPERATOR;
		token.str.push_back(ch);
		return token;
	}
	ss.putback(ch);

	return consume_value();
}

expr_token compiler::get_comp()
{
	char ch;
	ss >> ch;
	if (!ss || (ch != '=' && ch != '<' && ch != '>'))
		throw error::syntax_error();
	expr_token v;
	v.type = expr_token::OPERATOR;
	v.str.push_back(ch);
	return v;
}

bool compiler::pass_blank()
{
	int ch;
	while (1) {
		ch = ss.get();
		if (ch == std::istringstream::traits_type::eof())
			return false;
		if (!std::isblank(ch)) {
			ss.putback(ch);
			return true;
		}
	}
}

bool compiler::precedence_le(const expr_token& lhs, const expr_token& rhs)
{
	auto is_addsub = [](const expr_token& t) {
		return t.str == "+" || t.str == "-";
	};
	auto is_muldiv = [](const expr_token& t) {
		return t.str == "*" || t.str == "/";
	};
	return is_addsub(lhs) || is_muldiv(rhs);
}

void compiler::command_end()
{
	if (comm.type == command::BASIC_LET ||
			comm.type == command::BASIC_PRINT) {
		if (ss)
			throw error::syntax_error();
		return;
	}
	if (!ss)
		throw error::syntax_error();
	if (comm.type == command::BASIC_REM)
		return;
	char ch;
	ss >> ch;
	if (ss)
		throw error::syntax_error();
}

} // namespace BASIC
