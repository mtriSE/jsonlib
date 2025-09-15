#pragma once

#include <variant>
#include <vector>
#include <unordered_map>
#include <string>

namespace jsonlib
{
    struct value;

    using array = std::vector<value>;
    using object = std::unordered_map<std::string, value>;

    struct value
    {
        using number_t = double;
        std::variant<std::nullptr_t, bool, number_t, std::string, array, object> data;

        value() : data(nullptr) {}
        value(std::nullptr_t) : data(nullptr) {}
        value(bool b) : data(b) {}
        value(double n) : data(n) {}
        value(const std::string s) : data(std::move(s)) {}
        value(const char *s) : data(std::string(s)) {}
        value(array a) : data(std::move(a)) {}
        value(object o) : data(std::move(o)) {}

        // Helper methods
        bool is_null() const { return std::holds_alternative<std::nullptr_t>(data); }
        bool is_bool() const { return std::holds_alternative<bool>(data); }
        bool is_number() const { return std::holds_alternative<number_t>(data); }
        bool is_string() const { return std::holds_alternative<std::string>(data); }
        bool is_array() const { return std::holds_alternative<array>(data); }
        bool is_object() const { return std::holds_alternative<object>(data); }
    };

} // namespace jsonlib
