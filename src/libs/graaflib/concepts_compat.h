#ifndef CONCEPTS_COMPAT_H
#define CONCEPTS_COMPAT_H

// C++20 introduced 'concepts' which provide better template constraint checking
// and clearer error messages. However, older compilers (like GCC 8.x) don't support
// concepts. This header provides compatibility macros to allow the same code to work
// with both C++17 (using SFINAE) and C++20 (using concepts).

// Remove when no longer need support for Qt 5.15 and MinGW 8.1.

// Check if we have C++20 concepts support
// __cplusplus >= 202002L: Checks if C++20 or later is enabled
// __cpp_concepts: Feature-test macro that confirms concepts are actually implemented
#if __cplusplus >= 202002L && defined(__cpp_concepts)
#include <concepts>

// GRAAF_REQUIRES: Expands to a 'requires' clause in C++20
// This provides clean, readable template constraints with better error messages
#define GRAAF_REQUIRES(...) requires __VA_ARGS__

// GRAAF_ENABLE_IF: Does nothing in C++20 since we use 'requires' instead
// This macro expands to empty, avoiding duplicate constraints
#define GRAAF_ENABLE_IF_H(...)
#define GRAAF_ENABLE_IF(...)

#define GRAAF_RETTYPE(type, ...) type
#else
#include <type_traits>

// GRAAF_REQUIRES: Does nothing in C++17 since 'requires' doesn't exist
// The constraints are handled by GRAAF_ENABLE_IF instead
#define GRAAF_REQUIRES(...)

// GRAAF_ENABLE_IF: Expands to SFINAE using std::enable_if_t in C++17
// This is the traditional C++17 way to constrain templates
// __VA_ARGS__ contains the boolean expression that must be true
// The unnamed template parameter (typename =) triggers substitution failure
// if the condition is false, effectively constraining the template
// Note: The comma before this macro is important - it separates template parameters
#define GRAAF_ENABLE_IF_H(...) , std::enable_if_t<(__VA_ARGS__)> * = nullptr
#define GRAAF_ENABLE_IF(...) , std::enable_if_t<(__VA_ARGS__)> *

#define GRAAF_RETTYPE(type, ...) std::enable_if_t<(__VA_ARGS__), type>
#endif

#endif // CONCEPTS_COMPAT_H
