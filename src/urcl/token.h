#ifndef TOKEN_H
#define TOKEN_H

#include <string>

namespace urcl {
    class token {
        public:
            enum types_t {
                instruction,
                macro,
                name,
                symbol,
                label,
                relative,
                literal,
                real,
                reg,
                character,
                string,
                escape,
                mem,
                constant,
                comparison,
                bracket,
                comment,
                port
            } type;
            std::string original;
            std::string strVal;
            union values {
                long double real;
                int64_t literal;
                uint64_t mem;
                int32_t relative;
                uint32_t reg;
                wchar_t character;
            } value;
            std::string parse_error;
            std::string parse_warning;
            uint32_t column;

            
    };
}

#endif