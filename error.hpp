#ifndef BASIC_ERROR_HPP
#define BASIC_ERROR_HPP 1

namespace BASIC {
namespace error {

struct basic_error : public std::runtime_error {
	using std::runtime_error::runtime_error;
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
#ifdef LAB2_STYLE
"DIVIDE BY ZERO"
#else
"DIVISION BY ZERO"
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
} // namespace BASIC

#endif // BASIC_ERROR_HPP
