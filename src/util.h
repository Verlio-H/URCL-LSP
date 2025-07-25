#ifndef UTIL_H
#define UTIL_H

#include <string>

namespace util {
    size_t utf8len(const char* str);

    bool isWhitespace(char character);

    bool isodigit(char c);

    bool isbdigit(char c);

    bool isfdigit(char c);

    bool isNumber(const std::string& s);

    bool isOctal(const std::string& s);

    bool isBinary(const std::string& s);

    bool isHex(const std::string& s);

    bool isFloat(const std::string& s);

    std::string trim(const std::string& str);

    void replaceAll(std::string& str, const std::string& from, const std::string& to);

    std::string strToLower(std::string data);

    std::string strToUpper(std::string data);

    std::string to_utf8(char32_t codepoint);

    uint32_t from_utf8(std::string input);

    long double irisToFloat(uint16_t input);

    uint16_t floatToIris(long double input);

    std::string intHover(int64_t numb, uint32_t bits, bool iris);
}

#endif