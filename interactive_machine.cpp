#include "interactive_machine.hpp"

#include "error.hpp"

namespace BASIC {

integer_t interactive_machine::input_number()
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
			ss >> result;
			if (!ss)
				throw error::invalid_number();
			char ch;
			ss >> ch;
			if (ss)
				throw error::invalid_number();
			break;
		} catch (error::invalid_number& e) {
			std::cout << e.what() << std::endl;
		}
	}
	return result;
}

void interactive_machine::print_number(integer_t num)
{
	std::cout << num << std::endl;
}

} // namespace BASIC
