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

void interactive_machine::print_number(integer_t num)
{
	std::cout << num << std::endl;
}

} // namespace BASIC
