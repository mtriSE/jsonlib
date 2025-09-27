#include <iomanip>
#include <sstream>

#include "jsonlib/stringify.hpp"

namespace jsonlib {

namespace {

#pragma region HelperFunctions
/**
 * @brief Write a string to an output stream, escaping special characters.
 */
void write_string(std::ostream &output, const std::string &s);

/**
 * @brief Recursively dump a JSON value to an output stream.
 */
void dump(const value &v, std::ostream &os, bool pretty, int depth);

void write_string(std::ostream &output, const std::string &s) {
    output.put('"');
    for (unsigned char c : s) {
        switch (c) {
            case '"':
                output << "\\\"";
                break;
            case '\\':
                output << "\\\\";
                break;
            case '\b':
                output << "\\b";
                break;
            case '\f':
                output << "\\f";
                break;
            case '\n':
                output << "\\n";
                break;
            case '\r':
                output << "\\r";
                break;
            case '\t':
                output << "\\t";
                break;
            default:
                if (c < 0x20) {
                    output << "\\u"
                           << std::uppercase << std::hex
                           << std::setw(4) << std::setfill('0') << static_cast<int>(c)
                           << std::dec << std::nouppercase;
                } else {
                    output.put(static_cast<char>(c));  // normal character
                }
        }
    }
    output.put('"');
}

void dump(const value &v, std::ostream &output, bool pretty, int depth) {
    auto indent = [&](int d) {for(auto i = 0; i < d; i++) output << " "; };

    if (v.is_null()) {
        output << "null";
        return;
    }
    if (v.is_bool()) {
        output << (v.as_bool() ? "true" : "false");
        return;
    }
    if (v.is_number()) {
        output << v.as_number();
        return;
    }
    if (v.is_string()) {
        write_string(output, v.as_string());
        return;
    }

    if (v.is_array()) {
        const auto &arr = v.as_array();
        output << "[";

        if (!arr.empty()) {
            if (pretty) {
                output << "\n";
            }
            for (size_t i = 0; i < arr.size(); i++) {
                if (pretty) indent(depth + 2);
                dump(arr[i], output, pretty, depth + 2);
                if (i != arr.size() - 1) output << (pretty ? ",\n" : ",");
            }
            if (pretty) {
                output << "\n";
                indent(depth);
            }
        }

        output << "]";
        return;
    }

    if (v.is_object()) {
        const auto &obj = v.as_object();
        output << "{";

        if (!obj.empty()) {
            if (pretty) {
                output << "\n";
            }
            size_t count = 0;
            for (const auto &[key, val] : obj) {
                if (pretty) indent(depth + 2);
                write_string(output, key);
                output << (pretty ? ": " : ":");
                dump(val, output, pretty, depth + 2);
                if (count != obj.size() - 1) output << (pretty ? ",\n" : ",");
                count++;
            }
            if (pretty) {
                output << "\n";
                indent(depth);
            }
        }

        output << "}";
        return;
    }

}  // namespace
#pragma endregion HelperFunctions

}  // namespace

#pragma region PublicAPI
string to_string(const value &v, bool pretty /*= false*/) {
    std::ostringstream oss;
    dump(v, oss, pretty, 0);
    return oss.str();
}
#pragma endregion PublicAPI

}  // namespace jsonlib