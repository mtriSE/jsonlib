#pragma once
#include "value.hpp"

namespace jsonlib {
/**
 * @brief Convert a JSON value to a string. If pretty is true, the output will be
 *        formatted with indentation (2-space) and newlines.
 * @param v The JSON value to convert.
 * @param pretty Whether to pretty-print the output.
 * @return The JSON string representation of the value.
 */
string to_string(const value &v, bool pretty = false);

}  // namespace jsonlib