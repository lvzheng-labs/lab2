#ifndef BASIC_INTERACTIVE_MACHINE_HPP
#define BASIC_INTERACTIVE_MACHINE_HPP

#include "common.hpp"

#include "machine.hpp"

namespace BASIC {

struct interactive_machine : machine {
	virtual integer_t input_number() noexcept override;
	virtual void print_number(integer_t) noexcept override;
	virtual ~interactive_machine() override = default;
};

} // namespace BASIC

#endif // BASIC_INTERACTIVE_MACHINE_HPP
