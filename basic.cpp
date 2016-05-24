#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <experimental/optional>
#include <iostream>
#include <map>
#include <stack>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifndef NOT_LAB2_JUDGE
#define FUCK_LAB2
#endif

namespace BASIC {

template<class T>
using std_optional = std::experimental::optional<T>;

auto& std_nullopt = std::experimental::nullopt;

std::unordered_set<std::string> reserved_words = {
	"END",
	"GOTO",
	"IF",
	"INPUT",
	"LET",
	"PRINT",
	"REM",
	"THEN",
};

namespace error {

struct basic_error : public std::runtime_error {
	basic_error(std::string&& s):
		runtime_error{std::move(s)}
	{ }
};

struct empty_command : public basic_error {
	empty_command():
		basic_error{"EMPTY COMMAND"}
	{ }
};

struct end_of_file : public basic_error {
	end_of_file():
		basic_error{"END OF FILE"}
	{ }
};

struct divided_by_zero : public basic_error {
	divided_by_zero():
		basic_error{
#ifdef FUCK_LAB2
"DIVIDE BY ZERO"
#else
"DIVIDED BY ZERO"
#endif
}
	{ }
};

struct invalid_number : public basic_error {
	invalid_number():
		basic_error{"INVALID NUMBER"}
	{ }
};

struct variable_not_defined : public basic_error {
	variable_not_defined():
		basic_error{"VARIABLE NOT DEFINED"}
	{ }
};

struct line_number_error : public basic_error {
	line_number_error():
		basic_error{"LINE NUMBER ERROR"}
	{ }
};

struct syntax_error : public basic_error {
	syntax_error():
		basic_error{"SYNTAX ERROR"}
	{ }
};

} // namespace error

using integer_t = std::int64_t;
constexpr integer_t BASIC_INTEGER_MAX = INT64_MAX;
using short_t = std::int32_t;

using var_map_t = std::unordered_map<std::string, integer_t>;

// INT 0xff BAD_LINENO
const char asm_lang[][8] = {
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

class compiler {
public:
	command compile(const std::string& basic);

private:
	command comm;
	std::istringstream ss;

	std_optional<std::string> stored_keyword;

	void var_target();
	void let_equal();
	void shunting_yard_expr();
	void lineno_target();
	void if_condition();
	void if_then();

	std_optional<integer_t> consume_num();
	std_optional<std::string> consume_var();
	std_optional<expr_token> consume_value();
	std_optional<expr_token> consume_token();

	expr_token get_value();
	expr_token get_comp();

	bool pass_blank(); // return false if EOF is reached

	bool precedence_le(const expr_token& lhs, const expr_token& rhs);

	void command_end();
};

command compiler::compile(const std::string& basic)
{
	comm.clear();
	stored_keyword = std_nullopt;
	ss.clear();
	ss.seekg(0);
	ss.str(basic);
	std::string comm_str;
	ss >> comm_str;
	if (!ss)
		throw error::empty_command();
	if (comm_str == "REM") {
		comm.type = command::BASIC_REM;
	} else if (comm_str == "LET") {
		comm.type = command::BASIC_LET;
		var_target();
		let_equal();
		shunting_yard_expr();
	} else if (comm_str == "PRINT") {
		comm.type = command::BASIC_PRINT;
		shunting_yard_expr();
	} else if (comm_str == "INPUT") {
		comm.type = command::BASIC_INPUT;
		var_target();
	} else if (comm_str == "GOTO") {
		comm.type = command::BASIC_GOTO;
		lineno_target();
	} else if (comm_str == "IF") {
		comm.type = command::BASIC_IF;
		if_condition();
		if_then();
		lineno_target();
	} else if (comm_str == "END") {
		comm.type = command::BASIC_END;
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
			while (!operstack.empty() &&
					operstack.top().type == expr_token::OPERATOR &&
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
		if (ch == std::istringstream::traits_type::eof() || !std::isdigit(ch))
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
		if (ch == std::istringstream::traits_type::eof() || !std::isalnum(ch))
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

expr_token compiler::get_value()
{
	auto v = consume_value();
	if (!v)
		throw error::syntax_error();
	return *v;
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

using basic_code_t = std::map<std::size_t, std::string>;
using object_code_t = std::map<std::size_t, command>;
using binary_code_t = std::vector<instruction>;

struct machine {
	using var_pool_t = std::vector<std_optional<integer_t>>;
	using stack_t = std::stack<integer_t>;
	var_map_t var_map;
	var_pool_t vars;
	stack_t stack;
	struct registers {
		integer_t PC;
		integer_t STEP;
		integer_t STOP;
	} reg;
	void run(const binary_code_t& prog);
	void step(const instruction& ins);
	void clear();
	virtual integer_t input_number() noexcept = 0;
	virtual void print_number(integer_t) noexcept = 0;
	virtual ~machine() = default;
};

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

struct interactive_machine : machine {
	virtual integer_t input_number() noexcept override;
	virtual void print_number(integer_t) noexcept override;
	virtual ~interactive_machine() override = default;
};

integer_t interactive_machine::input_number() noexcept
{
	integer_t result;
	while (1) {
		std::cout << " ? ";
		std::string s;
		std::getline(std::cin, s);
		if (!std::cin)
			throw error::end_of_file();
		std::istringstream ss(s);
		try {
			try {
				ss >> result;
				if (!ss)
					throw error::invalid_number();
				char ch;
				ss >> ch;
				if (ss)
					throw error::invalid_number();
				break;
			} catch (std::invalid_argument&) {
				throw error::invalid_number();
			} catch (std::out_of_range&) {
				throw error::invalid_number();
			}
		} catch (error::invalid_number& e) {
			std::cout << e.what() << std::endl;
		}
	}
	return result;
}

void interactive_machine::print_number(integer_t num) noexcept
{
	std::cout << num << std::endl;
}

class linker {
public:
	linker(machine& mach):
		_mach(mach)
	{ }
	binary_code_t link(const object_code_t& obj);

private:
	machine& _mach;
	binary_code_t bin;
	struct lineno_to_link {
		std::size_t id_bin;
		std::size_t id_operand;
		std::size_t lineno;
	};
	std::vector<lineno_to_link> l2l;
	std::map<std::size_t, integer_t> lineno_map;

	void expand_expr(const expr_t& expr);
	void pop_to_var(const std::string& var);
	void program_print();
	void input_variable(const std::string& var);
	void program_goto(std::size_t lineno);
	void if_condition(const expr_t& exprl, const expr_t& exprr,
		const std::string& cmp, std::size_t lineno);
	void program_end();
	void push_number(const expr_token& token);

	integer_t get_var_addr(const std::string& var);
	short_t get_operator_op(const std::string& oper);
	void ask_lineno(std::size_t lineno);
	void linkall_lineno();
};

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

class interactive_console {
public:
	interactive_console();
	void run();
	void special_command(const std::string& s);
	void link();
	void run_program(const binary_code_t& prog);

private:
	basic_code_t _code;
	object_code_t _obj;
	compiler _comp;
	interactive_machine _vm;
	linker _ld;
	binary_code_t _prog;
	bool _prog_expire;
	bool _quit;
};

interactive_console::interactive_console():
	_ld(_vm),
	_prog_expire(true),
	_quit(false)
{
#if 0

#endif
}

void interactive_console::run()
{
	std::string s;
	while (!_quit) {
		while (std::isblank(std::cin.peek()))
			std::cin.get();
		std::getline(std::cin, s);
		if (!std::cin)
			break;
		if (s.empty())
			continue;
		std::size_t offset;
		std::size_t lineno;
		try {
			lineno = std::stoul(s, &offset);
		} catch (std::out_of_range&) {
			std::cout << "LINE NUMBER TOO LARGE" << std::endl;
			continue;
		} catch (std::invalid_argument&) {
			special_command(s);
			continue;
		}
		auto c = s.substr(offset);
		command comm;
		try {
			comm = _comp.compile(c);
		} catch (error::empty_command&) {
			_prog_expire = true;
			_code.erase(lineno);
			_obj.erase(lineno);
			continue;
		} catch (error::syntax_error& e) {
			std::cout << e.what() << std::endl;
			continue;
		}
		_prog_expire = true;
		_code[lineno] = std::move(c);
		_obj[lineno] = std::move(comm);
	}
}

void interactive_console::special_command(const std::string& s)
{
	std::istringstream ss(s);
	std::string c;
	ss >> c;
	char ch;
	try {
		if (c == "ASM") {
			if (ss >> ch)
				throw error::syntax_error();
			link();
			print_program(std::cout, _prog);
		} else if (c == "LIST") {
			for (auto& line : _code) {
				std::cout << line.first << line.second << std::endl;
			}
		} else if (c == "RUN") {
			if (ss >> ch)
				throw error::syntax_error();
			link();
			_vm.run(_prog);
		} else if (c == "HELP") {
			std::cout << "Sorry, not implemented." << std::endl;
		} else if (c == "QUIT") {
			if (ss >> ch)
				throw error::syntax_error();
			_quit = true;
		} else if (c == "INPUT" || c == "PRINT" || c == "LET") {
			object_code_t obj;
			obj[0] = _comp.compile(s);
			auto prog = _ld.link(obj);
			_vm.run(prog);
		} else if (c == "CLEAR") {
			_prog.clear();
			_prog_expire = true;
			_code.clear();
			_obj.clear();
			_vm.clear();
		} else {
			throw error::syntax_error();
		}
	} catch (error::basic_error& e) {
		std::cout << e.what() << std::endl;
	}
}

void interactive_console::link()
{
	if (_prog_expire) {
		_prog = _ld.link(_obj);
		_prog_expire = false;
	}
}

} // namespace BASIC

int main()
{
	BASIC::interactive_console console;
	console.run();
	return 0;
}
