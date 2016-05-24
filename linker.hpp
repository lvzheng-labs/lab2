#ifndef BASIC_LINKER_HPP
#define BASIC_LINKER_HPP

#include "common.hpp"

#include "command.hpp"
#include "machine.hpp"

namespace BASIC {

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

} // namespace BASIC

#endif // BASIC_LINKER_HPP
