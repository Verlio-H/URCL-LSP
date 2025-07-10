#include "util.h"

#include <cctype>
#include <algorithm>
#include <string>

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