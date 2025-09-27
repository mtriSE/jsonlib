#include <charconv>

#include "jsonlib/parse.hpp"

namespace jsonlib {
namespace {  // internal namespace
#pragma region Declarations
struct reader {
    std::string_view m_data;
    size_t index{0}, line{1}, column{1};

    /**
     * @brief Check if the end of the string has been reached.
     * @return true if the end of the string has been reached, false otherwise.
     */
    bool eof() const { return index >= m_data.size(); }

    /**
     * @brief Peek at the next character without consuming it.
     * @return The next character, or '\0' if the end of the string has been
     * reached.
     */
    char peek() const { return eof() ? '\0' : m_data[index]; }

    /**
     * @brief Get the next character and consume it.
     *        Updates line and column counters.
     * @return The next character.
     */
    char get() {
        // get character
        char c = peek();

        // update position
        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        index++;
        return c;
    }

    /**
     * @brief Skip whitespace characters (space, tab, newline, carriage return)
     *        and get the next character. If a non-whitespace character is
     *        encountered, stop skipping. Updates line and column counters.
     *
     */
    void skip_ws() {
        while (!eof()) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                get();
            else
                break;
        }
    }

    /**
     * @brief Expect the next character to be `ch`, otherwise throw a parse
     * error. Then consume the character.
     * @param ch The expected character.
     * @throws `parse_error` if the next character is not `ch`.
     */
    void expect(char ch) {
        if (peek() != ch) {
            fail("Unexpected character");
        }
        get();
    }

    [[noreturn]] void fail(const char* msg) const {
        parse_error e(msg);
        e.line = line;
        e.column = column;
        e.offset = index;
        throw e;
    }
};

using ::jsonlib::array;
using ::jsonlib::number_t;
using ::jsonlib::object;
using ::jsonlib::string;
using ::jsonlib::value;

string parse_string(reader& r);
number_t parse_number(reader& r);
value parse_array(reader& r);
value parse_object(reader& r);
value parse_value(reader& r);  // call all parse type-specific functions

value parse(const std::string_view src) { return "Parsed JSON"; }
#pragma endregion Declarations

#pragma region Definitions
/**
 * @brief Parse a JSON string value.
 * @param r The `reader` to parse from.
 * @return The parsed string.
 * @throws `parse_error` if the string is not valid.
 */
string parse_string(reader& r) {
    r.expect('"');
    string out;
    while (!r.eof()) {
        char c = r.peek();
        if (c == '"') {
            r.get();
            return out;
        }
        if (c == '\\') {  // handle escape sequences
            char e = r.get();
            switch (e) {
                case '"':
                    out.push_back('"');
                    break;
                case '\\':
                    out.push_back('\\');
                    break;
                case '/':
                    out.push_back('/');
                    break;
                case 'b':
                    out.push_back('\b');
                    break;
                case 'f':
                    out.push_back('\f');
                    break;
                case 'n':
                    out.push_back('\n');
                    break;
                case 'r':
                    out.push_back('\r');
                    break;
                case 't':
                    out.push_back('\t');
                    break;
                case 'u': {
                    /** Unicode escape sequence \uXXXX
                     * Reference: https://en.wikipedia.org/wiki/List_of_Unicode_characters
                     */
                    unsigned code = 0;

                    // read 4 hex digits and cumulate the code point
                    for (auto i : {0, 1, 2, 3}) {
                        char digit = r.get();

                        // shift left by 4 bits.
                        // Because each hexadecimal digit represents 4 bits (0x0 - 0xF(15))
                        code <<= 4;

                        if (digit >= '0' && digit <= '9')
                            code |= (digit - '0');
                        else if (digit >= 'a' && digit <= 'f')
                            code |= (digit - 'a' + 10);
                        else if (digit >= 'A' && digit <= 'F')
                            code |= (digit - 'A' + 10);
                        else
                            r.fail("Invalid Unicode escape sequence");
                    }

                    /*
                     * Code point to UTF-8 conversion table
                     *
                     * UTF-8 encodes code points in one to four bytes, depending
                     * on the value of the code point. In the following table,
                     * the characters u to z are replaced by the bits of the
                     * code point, from the positions U+uvwxyz:
                     * code point = 0b ----'----'uuuu'vvvv'wwww'xxxx'yyyy'zzzz
                     *
                     *
                     * |First   |Last    |Byte 1  |Byte 2  |Byte 3  |Byte 4  |
                     * |code    |code    |--------|--------|--------|--------|
                     * |point   |point   |--------|--------|--------|--------|
                     * |--------|--------|--------|--------|--------|--------|
                     * |U+0000  |U+007F  |0yyyyyyy|        |        |        |
                     * |U+0080  |U+07FF  |110xxxyy|10yyzzzz|        |        |
                     * |U+0800  |U+FFFF  |1110wwww|10xxxxyy|10yyzzzz|        |
                     * |U+010000|U+10FFFF|11110uvv|10vvwwww|10xxxxyy|10yyzzzz|
                     */
                    if (code <= 0x7F) {                          // 1-byte sequence (control code) - C0
                        out.push_back(static_cast<char>(code));  // single byte: 0yyyyyyy

                    } else if (code <= 0x7FF) {                                  // 2-byte sequence (0x80 - 0x7FF)
                        out.push_back(static_cast<char>(0xC0 | (code >> 6)));    // first byte: 110xxxyy
                        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));  // second byte: 10yyzzzz

                    } else if (code <= 0xFFFF) {                                      // 3-byte sequence (0x800 - 0xFFFF)
                        out.push_back(static_cast<char>(0xE0 | (code >> 12)));        // first byte: 1110wwww
                        out.push_back(static_cast<char>(0x80 | (code >> 6 & 0x3F)));  // second byte: 10xxxxyy
                        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));       // third byte: 10yyzzzz

                    } else if (code <= 0x10FFFF) {                                          // 4-byte sequence (0x10000 - 0x10FFFF)
                        out.push_back(static_cast<char>(0xF0 | (code >> (4 * 4 + 2))));     // first byte: 11110uvv
                        out.push_back(static_cast<char>(0x80 | (code >> (3 * 4) & 0x3F)));  // second byte: 10vvwwww
                        out.push_back(static_cast<char>(0x80 | (code >> (4 + 2) & 0x3F)));  // third byte: 10xxxxyy
                        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));             // fourth byte: 10yyzzzz
                    } else {
                        // invalid code point
                        r.fail("Invalid Unicode code point, out of range.");
                    }
                    break;
                }
                default:
                    r.fail("Invalid escape sequence");
            }
        } else {
            // regular character
            if (static_cast<unsigned char>(c) < 0x20) {
                r.fail("Control characters must be escaped in JSON strings");
            }
            out.push_back(c);
            r.get();
        }
    }
    r.fail("Unterminated string");
}

/**
 * @brief Parse a JSON number value.
 * @param r The `reader` to parse from.
 * @return The parsed number.
 * @throws `parse_error` if the number is not valid.
 */
number_t parse_number(reader& r) {
    size_t start = r.index;
    if (r.peek() == '-') r.get();
    if (r.peek() == '0') {
        r.get();
    } else if (r.peek() >= '1' && r.peek() <= '9') {
        do {
            r.get();
        } while (r.peek() >= '0' && r.peek() <= '9');
    } else {
        r.fail("Invalid number");
    }

    if (r.peek() == '.') {
        r.get();
        if (!(r.peek() >= '0' && r.peek() <= '9')) {
            r.fail("Invalid fraction");
        }
        do {
            r.get();
        } while (r.peek() >= '0' && r.peek() <= '9');
    }

    if (r.peek() == 'e' || r.peek() == 'E') {
        r.get();
        if (r.peek() == '+' || r.peek() == '-') {
            r.get();
        }
        if (!(r.peek() >= '0' && r.peek() <= '9')) {
            r.fail("Invalid exponent");
        }
        do {
            r.get();
        } while (r.peek() >= '0' && r.peek() <= '9');
    }

    number_t out = 0.0;
    auto sv = r.m_data.substr(start, r.index - start);
    std::from_chars(sv.data(), sv.data() + sv.size(), out);
    return out;
}

/**
 * @brief Parse a JSON array value.
 * @param r The `reader` to parse from.
 * @return The parsed array.
 * @throws `parse_error` if the array is not valid.
 */
value parse_array(reader& r) {
    r.expect('[');
    r.skip_ws();
    array arr;
    if (r.peek() == ']') {
        r.get();
        return value(std::move(arr));
    }
    while (true) {
        r.skip_ws();
        arr.push_back(parse_value(r));
        r.skip_ws();
        if (r.peek() == ',') {
            r.get();
            continue;
        }
        if (r.peek() == ']') {
            r.get();
            break;
        }
        r.fail("Expected ',' or ']'");
    }
    return value(std::move(arr));
}

/**
 * @brief Parse a JSON object value.
 * @param r The `reader` to parse from.
 * @return The parsed object.
 * @throws `parse_error` if the object is not valid.
 */
value parse_object(reader& r) {
    r.expect('{');
    r.skip_ws();
    object obj;
    if (r.peek() == '}') {
        r.get();
        return value(std::move(obj));
    }
    while (true) {
        r.skip_ws();
        if (r.peek() != '"') r.fail("Expected string key");
        auto key = parse_string(r);
        r.skip_ws();
        r.expect(':');
        r.skip_ws();
        obj.emplace(std::move(key), parse_value(r));
        r.skip_ws();
        if (r.peek() == ',') {
            r.get();
            continue;
        }
        if (r.peek() == '}') {
            r.get();
            break;
        }
        r.fail("Expected ',' or '}'");
    }
}

/**
 * @brief Parse a JSON value (string, number, object, array, true, false, null).
 * @param r The `reader` to parse from.
 * @return The parsed value.
 * @throws `parse_error` if the value is not valid.
 */
value parse_value(reader& r) {
    r.skip_ws();
    char c = r.peek();
    if (c == '"') {
        return value(parse_string(r));
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        return value(parse_number(r));
    }
    if (c == '{') {
        return parse_object(r);
    }
    if (c == '[') {
        return parse_array(r);
    }
    if (c == 't' && r.m_data.substr(r.index, 4) == "true") {
        r.index += 4;
        r.column += 4;
        return value(true);
    }
    if (c == 'f' && r.m_data.substr(r.index, 5) == "false") {
        r.index += 5;
        r.column += 5;
        return value(false);
    }
    if (c == 'n' && r.m_data.substr(r.index, 4) == "null") {
        r.index += 4;
        r.column += 4;
        return value(nullptr);
    }
    r.fail("Unexpected token");
}
#pragma endregion Definitions
}  // namespace

#pragma region PublicAPI
value parse(const std::string_view src) {
    reader r{src};
    value v = parse_value(r);
    r.skip_ws();
    if (!r.eof()) {
        /**
         * there are characters or data present in the input stream
         * after a complete and valid JSON object or array has been parsed
         */
        r.fail("Unexpected trailing characters");
    }
    return v;
}
#pragma endregion PublicAPI

}  // namespace jsonlib