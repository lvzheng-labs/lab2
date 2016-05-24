#ifndef BASIC_COMMON_HPP
#define BASIC_COMMON_HPP

#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <experimental/optional>
#include <iostream>
#include <map>
#include <stack>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifndef NOT_LAB2_JUDGE
#define FUCK_LAB2
#else
#define BASIC_ENABLE_EXTENSIONS
#endif

namespace BASIC {

// std::optional

template<class T>
using std_optional = std::experimental::optional<T>;

static auto& std_nullopt = std::experimental::nullopt;

// reserved words

static const std::unordered_set<std::string> reserved_words = {
	"END",
	"GOTO",
	"IF",
	"INPUT",
	"LET",
	"PRINT",
	"REM",
	"THEN",
};

using integer_t = std::int64_t;
constexpr integer_t BASIC_INTEGER_MAX = INT64_MAX;
using short_t = std::int32_t;

using basic_code_t = std::map<std::size_t, std::string>;

} // namespace BASIC

#endif // BASIC_COMMON_HPP
