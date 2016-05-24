#include "interactive_console.hpp"

#include "error.hpp"

namespace BASIC {

#ifdef BASIC_ENABLE_EXTENSIONS
static void print_program(std::ostream& os, binary_code_t& prog)
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
#endif // BASIC_ENABLE_EXTENSIONS

interactive_console::interactive_console():
	_ld(_vm),
	_prog_expire(true),
	_quit(false)
{ }

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
#ifdef BASIC_ENABLE_EXTENSIONS
		if (c == "ASM") {
			if (ss >> ch)
				throw error::syntax_error();
			link();
			print_program(std::cout, _prog);
		} else
#endif // BASIC_ENABLE_EXTENSIONS
		if (c == "CLEAR") {
			_prog.clear();
			_prog_expire = true;
			_code.clear();
			_obj.clear();
			_vm.clear();
		} else if (c == "HELP") {
			std::cout << "Sorry, not implemented." << std::endl;
		} else if (c == "LIST") {
			for (auto& line : _code) {
				std::cout << line.first
					<< line.second << std::endl;
			}
		} else if (c == "QUIT") {
			if (ss >> ch)
				throw error::syntax_error();
			_quit = true;

		} else if (c == "RUN") {
			if (ss >> ch)
				throw error::syntax_error();
			link();
			_vm.run(_prog);
		} else if (c == "INPUT" || c == "PRINT" || c == "LET") {
			object_code_t obj;
			obj[0] = _comp.compile(s);
			auto prog = _ld.link(obj);
			_vm.run(prog);
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
