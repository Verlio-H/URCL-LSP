#include "util.h"

#include <cctype>
#include <algorithm>
#include <string>
#include <format>

size_t util::utf8len(const char* str) {
    size_t len = 0;
    for (size_t i = 0; *str != 0; ++len) {
        int v01 = ((*str & 0x80) >> 7) & ((*str & 0x40) >> 6);
        int v2 = (*str & 0x20) >> 5;
        int v3 = (*str & 0x10) >> 4;
        str += 1 + ((v01 << v2) | (v01 & v3));
    }
    return len;
}

bool util::isWhitespace(char character) {
    return character == ' ' || character == '\t' || character == '\r';
}

bool util::isodigit(char c) {
    return (c >= '0' && c <= '7');
}

bool util::isbdigit(char c) {
    return c == '0' || c == '1';
}

bool util::isfdigit(char c) {
    return isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+';
}

bool util::isNumber(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), isdigit);
}

bool util::isOctal(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), util::isodigit);
}

bool util::isBinary(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), util::isbdigit);
}

bool util::isHex(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), isxdigit);
}

bool util::isFloat(const std::string& s) {
    if (s.find('.') != s.rfind('.')) return false;
    if (s.find_first_of("eE") != s.find_last_of("eE")) return false;
    if (s.find_first_of("-+") != s.find_last_of("-+")) return false;
    if (s.find_first_of("-+") != std::string::npos && s.find_first_of("-+") != s.find_first_of("Ee") + 1) return false;
    if (s.find_first_of("eE") != std::string::npos && s.find('.') > s.find_first_of("eE")) return false;
    return !s.empty() && std::all_of(s.begin(), s.end(), util::isfdigit);
}

std::string util::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");

    if (std::string::npos == first) {
        return "";
    }

    size_t last = str.find_last_not_of(" \t");

    return str.substr(first, (last - first + 1));
}

void util::replaceAll(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

std::string util::strToLower(std::string data) {
    std::transform(data.begin(), data.end(), data.begin(), tolower);
    return data;
}

std::string util::strToUpper(std::string data) {
    std::transform(data.begin(), data.end(), data.begin(), toupper);
    return data;
}


std::string util::to_utf8(char32_t codepoint) {
    char array[5];
    if (codepoint <= 0x7F) {
        array[0] = (char)codepoint;
        array[1] = 0;
    } else if (codepoint <= 0x7FF) {
        array[0] = (char)(codepoint >> 6) | 0xC0;
        array[1] = ((char)(codepoint) & 0x3F) | 0x80;
        array[2] = 0;
    } else if (codepoint <= 0xFFFF) {
        array[0] = (char)(codepoint >> 12) | 0xE0;
        array[1] = ((char)(codepoint >> 6) & 0x3F) | 0x80;
        array[2] = ((char)(codepoint) & 0x3F) | 0x80;
        array[3] = 0;
    } else if (codepoint <= 0x10FFFF) {
        array[0] = (char)(codepoint >> 18) | 0xF0;
        array[1] = ((char)(codepoint >> 12) & 0x3F) | 0x80;
        array[2] = ((char)(codepoint >> 6) & 0x3F) | 0x80;
        array[3] = ((char)(codepoint) & 0x3F) | 0x80;
        array[4] = 0;
    } else {
        return "�";
    }
    return std::string(array);
}

uint32_t util::from_utf8(std::string input) {
    uint32_t result;
    if ((unsigned char)input[0] >= 0xF0) { // 4 byte
        result = (input[0] & 0x7) << 18;
        result += (input[1] & 0x3F) << 12;
        result += (input[2] & 0x3F) << 6;
        result += (input[3] & 0x3F);
    } else if ((unsigned char)input[0] >= 0xE0) { // 3 byte
        result = (input[0] & 0xF) << 12;
        result += (input[1] & 0x3F) << 6;
        result += (input[2] & 0x3F);
    } else if ((unsigned char)input[0] >= 0xC0) { // 2 byte
        result = (input[0] & 0x1F) << 6;
        result += (input[1] & 0x3F);
    } else {
        result = input[0];
    }
    return result;
}

long double util::irisToFloat(uint16_t input) {
    if (input == 0 || input == 0xFFFF) return 0;
    long double sign = (input & 0x8000) != 0 ? -1.0l : 1.0l;
    if (sign < 0) input = ~input;
    int16_t exponent = ((input >> 10) & 0x1F) - 16;
    uint16_t mantissa = input & 0x3FF;
    return ((long double)mantissa / 0x400 + 1) * powl(2, exponent) * sign;
}

uint16_t util::floatToIris(long double input) {
    if (input == 0) return 0;
    int16_t exponent = 0;
    int16_t sign = input < 0;
    if (sign) input = fabsl(input);
    while (std::roundl(input * 0x400) / 0x400 >= 2.0) {
        input /= 2;
        if (++exponent > 15) return 0x7FFF + sign;
    }
    while (std::roundl(input * 0x400) / 0x400 < 1.0) {
        input *= 2;
        if (--exponent < -16) return -sign;
    }
    input -= 1;
    uint16_t result = ((exponent + 16) << 10) + (uint16_t)std::roundl(input * 0x400);
    return sign ? ~result : result;
}

std::string util::intHover(int64_t numb, uint32_t bits, bool iris) {
    std::string result;
    if (numb <= 0xFFFFFFFF && numb >= 32 && numb != '\'') {
        char buf[5]{};
        result = "\'" + util::to_utf8(numb) + "\'\\\n";
    } else if (numb < 32 || numb == '\'') {
        switch (numb) {
            case ('\''):
                result = "'\\''\\\n";
            case ('\n'):
                result = "'\\n'\\\n";
            case ('\r'):
                result = "'\\r'\\\n";
            case ('\t'):
                result = "'\\t'\\\n";
            case ('\b'):
                result = "'\\b'\\\n";
            case ('\f'):
                result = "'\\f'\\\n";
            case ('\v'):
                result = "'\\v'\\\n";
            case ('\0'):
                result = "'\\0'\\\n";
            default:
                break;
        }
    }
    uint64_t mask = bits >= 64 ? -1 : ((uint64_t)1 << bits) - 1;
    result += std::format("{0:} ({1:})\\\n0x{1:x}\\\n0o{1:o}\\\n0b{1:b}", numb, (uint64_t)numb & mask);
    if (bits == 16 && iris) {
        result += std::format("\\\n{}", util::irisToFloat(numb));
    } else if (bits == 32 && sizeof(float) == sizeof(uint32_t)) {
        uint32_t shortenedVal = numb;
        result += std::format("\\\n{}", reinterpret_cast<float &>(shortenedVal));
    } else if (bits == 64 && sizeof(double) == sizeof(int64_t)) {
        result += std::format("\\\n{}", reinterpret_cast<double &>(numb));
    }
    return result;
}