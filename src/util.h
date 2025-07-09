#ifndef UTIL_H
#define UTIL_H

#include <string>

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

#endif