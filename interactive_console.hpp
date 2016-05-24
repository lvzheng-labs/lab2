#ifndef BASIC_INTERACTIVE_CONSOLE_HPP
#define BASIC_INTERACTIVE_CONSOLE_HPP

#include "common.hpp"

#include "compiler.hpp"
#include "interactive_machine.hpp"
#include "linker.hpp"

namespace BASIC {

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

} // namespace BASIC

#endif // BASIC_INTERACTIVE_CONSOLE_HPP
