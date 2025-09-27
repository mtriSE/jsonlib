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
    using string = std::string;
    using number_t = double;

    struct value
    {
        std::variant<std::nullptr_t, bool, number_t, string, array, object> data;

        value() : data(nullptr) {}
        value(std::nullptr_t) : data(nullptr) {}
        value(bool b) : data(b) {}
        value(number_t n) : data(n) {}
        value(const string s) : data(std::move(s)) {}
        value(const char* s) : data(string(s)) {}
        value(array a) : data(std::move(a)) {}
        value(object o) : data(std::move(o)) {}

        // Helper methods
        bool is_null() const { return std::holds_alternative<std::nullptr_t>(data); }
        bool is_bool() const { return std::holds_alternative<bool>(data); }
        bool is_number() const { return std::holds_alternative<number_t>(data); }
        bool is_string() const { return std::holds_alternative<string>(data); }
        bool is_array() const { return std::holds_alternative<array>(data); }
        bool is_object() const { return std::holds_alternative<object>(data); }

        // Accessors
        bool& as_bool() { return std::get<bool>(data); }
        number_t& as_number() { return std::get<number_t>(data); }
        string& as_string() { return std::get<string>(data); }
        array& as_array() { return std::get<array>(data); }
        object& as_object() { return std::get<object>(data); }

        const bool& as_bool() const { return std::get<bool>(data); }
        const number_t& as_number() const { return std::get<number_t>(data); }
        const string& as_string() const { return std::get<string>(data); }
        const array& as_array() const { return std::get<array>(data); }
        const object& as_object() const { return std::get<object>(data); }
    };

} // namespace jsonlib
