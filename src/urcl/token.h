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
                port,
                uir
            } type;
            std::string original;
            std::string strVal;
            union values {
                long double real;
                int64_t literal;
            } value;
            std::string parse_error;
            std::string parse_warning;
            uint32_t column;
    };
}

#endif