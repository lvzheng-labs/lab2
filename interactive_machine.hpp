#ifndef BASIC_INTERACTIVE_MACHINE_HPP
#define BASIC_INTERACTIVE_MACHINE_HPP

#include "common.hpp"

#include "machine.hpp"

namespace BASIC {

// input: read from stdin with hint " ? " until success
// print: println
struct interactive_machine : machine {
	virtual integer_t input_number() override;
	virtual void print_number(integer_t) override;
	virtual ~interactive_machine() override = default;
};

} // namespace BASIC

#endif // BASIC_INTERACTIVE_MACHINE_HPP
