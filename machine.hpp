#ifndef BASIC_MACHINE_HPP
#define BASIC_MACHINE_HPP

#include "common.hpp"

#include "instruction.hpp"

namespace BASIC {

class machine {
private:
	using var_map_t = std::unordered_map<std::string, integer_t>;
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
	void step(const instruction& ins);
protected:
	virtual integer_t input_number() noexcept = 0;
	virtual void print_number(integer_t) noexcept = 0;
public:
	void run(const binary_code_t& prog);
	void clear();
	virtual ~machine() = default;

	friend class linker;
};

} // namespace BASIC

#endif // BASIC_MACHINE_HPP
