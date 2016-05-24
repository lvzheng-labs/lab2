# Naive BASIC Virtual Machine

This is my implementation of SJTU 2016 Spring SE `lab2` assignment.

## Features

 - It runs very fast, faster than Python for simple programs such as computing
   PI(10000)=1229. It runs even faster than Hong Kong journalists.
 - Though dynamic language, it runs in an as static style as possible.
 - Written in C++17, with no other dependencies.
 - Tiny. Less than 2k lines of code.

## Build

Just `make`. If you are not running lab2 judging program and hope to run fast,
```sh
CXXFLAGS="-DNOT_LAB2_JUDGE -g" make
```
This allows ASM command to print assembly code and change some small UI words.

Your C++ library should have a working `<experimental/optional.hpp>`.  If your
compiler does not speak c++17, change CXXSTDFLAGS to -std=c++14, or edit
`common.hpp` to use `boost::optional` instead.

If your C++ library is new enough to have `<optional.hpp>`, edit `common.hpp`
to use `std::optional`.

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
