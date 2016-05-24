# Naive BASIC Virtual Machine

This is my implementation of SJTU 2016 Spring SE `lab2` assignment.

## Build

Just `make`. If you are not running lab2 judging program,
```sh
CXXFLAGS=-DNOT_LAB2_JUDGE make
```

## Dependencies

c++1z with `<experimental/optional.hpp>`.

If your compiler does not speak c++1z, change CXXSTDFLAGS to -std=c++14.

## How does it work

### Compile: BASIC code -> parsed code (object code)

BASIC code is `compile`d to object code line by line, with expressions stored
as RPN, and variables and line numbers unstripped.

### Link: parsed code -> machine code (binary instructions)

Turn parsed code to binary code that can be run directly by a given instance of
BASIC VM. Variables and line numbers are linked and stripped. Name-index map of
variables are stored in the given VM.

### Run

Run the machine code in VM emulator.

## Author

Lv Zheng <lv.zheng.2015@gmail.com>
