#pragma once
#include <stdexcept>
#include "value.hpp"

namespace jsonlib
{

    struct parse_error : public std::runtime_error
    {
        size_t line{}, column{}, offset{};
        using std::runtime_error::runtime_error; // inherit constructors
    };

    value parse(const std::string_view src); // throws parse_error on failure
    
} // namespace jsonlib
