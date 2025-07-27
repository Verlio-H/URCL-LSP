#include "source.h"
#include "../util.h"
#include "defines.h"

#include <cctype>
#include <fstream>
#include <lsp/types.h>
#include <string>
#include <clocale>
#include <cuchar>

urcl::source::source() {}

urcl::source::source(const std::vector<std::string>& source, const urcl::config& config) {
    this->code.reserve(source.size());

    // init macros
    macros = {"@DEFINE"};
    if (config.useUrcx) {
        macros.insert(urcl::defines::URCX_MACROS.begin(), urcl::defines::URCX_MACROS.end());
    }

    // init ports
    ports = urcl::defines::STD_PORTS;
    if (config.useUrcx) {
        ports.insert(urcl::defines::URCX_PORTS.begin(), urcl::defines::URCX_PORTS.end());
    }
    if (config.useIris) {
        ports.insert(urcl::defines::IRIS_PORTS.begin(), urcl::defines::IRIS_PORTS.end());
    }

    // init instructions
    instructions = urcl::defines::HEADERS;
    instructions.insert(urcl::defines::OTHER_INSTRUCTIONS.begin(), urcl::defines::OTHER_INSTRUCTIONS.end());
    if (config.useCore) {
        instructions.insert(urcl::defines::CORE_INSTRUCTIONS.begin(), urcl::defines::CORE_INSTRUCTIONS.end());
    }
    if (config.useBasic) {
        instructions.insert(urcl::defines::BASIC_INSTRUCTIONS.begin(), urcl::defines::BASIC_INSTRUCTIONS.end());
        if (config.useUrcx || !config.useIris) {
            instructions.insert(urcl::defines::STACK_INSTRUCTIONS.begin(), urcl::defines::STACK_INSTRUCTIONS.end());
        }
    }
    if (config.useComplex) {
        instructions.insert(urcl::defines::COMPLEX_INSTRUCTIONS.begin(), urcl::defines::COMPLEX_INSTRUCTIONS.end());
    }
    if (config.useIris) {
        instructions.insert(urcl::defines::IRIS_INSTRUCTIONS.begin(), urcl::defines::IRIS_INSTRUCTIONS.end());
    }
    if (config.useUrcx) {
        instructions.insert(urcl::defines::URCX_INSTRUCTIONS.begin(), urcl::defines::URCX_INSTRUCTIONS.end());
    }
    if (config.useUrcx && config.useIris) {
        instructions.insert(urcl::defines::IRIX_INSTRUCTIONS.begin(), urcl::defines::IRIX_INSTRUCTIONS.end());
    }

    bool inComment = false;
    for (const std::string& line : source) {
        this->code.emplace_back(parseLine(line, inComment, config));
    }
}

void urcl::source::updateReferences(const std::unordered_map<std::filesystem::path, urcl::source>& all, const urcl::config& config) {
    for (std::filesystem::path path : config.includes) {
        bool found = false;
        for (std::pair<std::filesystem::path, urcl::source> loaded : all) {
            if (std::filesystem::exists(path) && std::filesystem::equivalent(path, loaded.first)) {
                found = true;
                includes.emplace(path, loaded.second);
            }
        }
        if (found) continue;
        std::vector<std::string> document;
        std::ifstream in(path);
        
        std::string line;
        while (std::getline(in, line)) {
            document.emplace_back(std::move(line));
        }

        urcl::source fileSrc(document, config);
        includes.emplace(path, fileSrc);
    }
}

void urcl::source::updateDefinitions(const std::filesystem::path& loc) {
    updateDefinitions(*this, loc, true);
}

void urcl::source::updateDefinitions(urcl::source& code, const std::filesystem::path& loc, bool base) {
    urcl::object_id nextObjId = 1;
    urcl::object_id currentObjId = 0;
    urcl::source& source = *this;
    for (int i = 0; i < code.code.size(); ++i) {
        std::vector<urcl::token>& line = code.code[i];
        bool exit = false;
        for (int k = 0; k < line.size(); ++k) {
            urcl::token& token = line[k];
            if (exit) break;
            switch (token.type) {
                case (urcl::token::macro):
                    exit = true;
                    if (token.strVal != "@DEFINE") break;
                    for (int j = k + 1; j < line.size(); ++j) {
                        if (line[j].type == urcl::token::comment) continue;
                        source.definesDefs[line[j].original] = {loc, i};
                        break;
                    }
                    break;
                case (urcl::token::symbol):
                    exit = true;
                    if (token.original.length() >= 3 && token.original.substr(0, 3) == "!!!") {
                        source.objectDefs.emplace_back(urcl::sub_object::open, i);
                        if (currentObjId != 0) {
                            token.parse_error = "Opened sub-object while already within one";
                        }
                        currentObjId = nextObjId++;
                    } else if (token.original.length() >= 2 && token.original.substr(0, 2) == "!!") {
                        source.objectDefs.emplace_back(urcl::sub_object::close, i);
                        if (currentObjId == 0) {
                            token.parse_error = "Closed sub-object without opening one";
                        }
                        currentObjId = 0;
                    } else {
                        source.symbolDefs[token.original] = {loc, i};
                    }
                    break;
                case (urcl::token::label):
                    exit = true;
                    if (!base) break;
                    if (source.labelDefs.contains(token.original)) {
                        if (token.parse_error == "") {
                            token.parse_error = "Label already defined";
                        }
                        break;
                    }
                    source.labelDefs[token.original] = {currentObjId, i};
                    break;
                case (urcl::token::instruction):
                    if (token.strVal != "BITS") {
                        exit = true;
                        break;
                    }
                    for (int j = k + 1; j < line.size(); ++j) {
                        if (line[j].type == urcl::token::comment) continue;
                        if (line[j].type == urcl::token::comparison) continue;
                        if (line[j].value.literal > UINT16_MAX) break;
                        bits = std::max(bits, (uint16_t)line[j].value.literal);
                        break;
                    }
                    exit = true;
                    break;
                case (urcl::token::comment):
                    break;
                default:
                    exit = true;
                    break;
            }
        }
    }
    if (!base) return;
    for (std::pair<const std::filesystem::path, urcl::source>& data : source.includes) {
        source.updateDefinitions(data.second, data.first, false);
    }
}

void urcl::source::updateErrors(const urcl::config& config) {
    // build constants
    std::unordered_set<std::string> constants = urcl::defines::STD_CONSTS;

    if (config.useUrcx) {
        constants.insert(urcl::defines::URCX_CONSTS.begin(), urcl::defines::URCX_CONSTS.end());
    }
    this->constants = std::move(constants);

    urcl::object_id nextObjId = 1;
    urcl::object_id currentObjId = 0;
    for (std::vector<urcl::token>& line : code) {
        bool expect = true;
        bool inArray = false;
        int operand = -1;
        uint32_t instColumn = 0;
        std::string_view inst;
        const std::vector<urcl::defines::op_type> *operands = nullptr;
        for (urcl::token& token : line) {
            if (inArray && token.type == urcl::token::bracket && token.original == "]") {
                inArray = false;
                continue;
            }

            if (token.type == urcl::token::comment) continue;

            if (!inArray) ++operand;
            if (operand == 0 && (token.type == urcl::token::label || token.type == urcl::token::symbol)) {
                if (token.original.length() >= 3 && token.original.substr(0, 3) == "!!!") {
                    currentObjId = nextObjId++;
                } else if (token.original.length() >= 2 && token.original.substr(0, 2) == "!!") {
                    currentObjId = 0;
                }
                expect = false;
                continue;
            }

            if (!operands && (token.type == urcl::token::instruction || token.type == urcl::token::macro)) {
                inst = std::string_view(token.strVal);
                instColumn = token.column;
                if (urcl::defines::INST_INFO.contains(token.strVal)) {
                    operands = &urcl::defines::INST_INFO.at(token.strVal).second;
                }
            }

            if (token.parse_error != "") continue;

            if (!inArray && token.type == urcl::token::bracket && token.original == "]") {
                token.parse_error = "Closing bracket before opening bracket";
                continue;
            }

            if (operand == 1 && inst == "OUT" && token.column == instColumn + 3) {
                if (urcl::defines::OUT_INFO.contains(token.strVal)) {
                    operands = &urcl::defines::OUT_INFO.at(token.strVal).second;
                }
            } else if (token.type == urcl::token::port && operand == 1 && inst == "IN" && (config.useUir || config.useIris || token.column == instColumn + 2)) {
                if (urcl::defines::IN_INFO.contains(token.strVal)) {
                    operands = &urcl::defines::IN_INFO.at(token.strVal).second;
                } else {
                    operands = &urcl::defines::IN_DEFAULT;
                }
            }

            if (!inArray && operands) {
                if (operands->size() < operand) {
                    if (inst != "@DEBUG") token.parse_error = "Too many operands in language construct";
                } else if (operand != 0) {
                    urcl::defines::op_type op = operands->at(operand - 1);
                    switch (op) {
                        case (urcl::defines::op_type::inst): {
                            if (token.type != urcl::token::instruction) {
                                token.parse_error = "Unexpected operand type";
                            }
                            break;
                        }
                        case (urcl::defines::op_type::port): {
                            if (token.type != urcl::token::port) {
                                token.parse_error = "Expected port in operand";
                            }
                            break;
                        }
                        case (urcl::defines::op_type::array): {
                            if (token.type == urcl::token::bracket && token.original == "[") break;
                            if (!tokenIsImmediate(token, *this) || (!config.useIris && token.original == "_")) {
                                token.parse_error = "Expected array type or immediate value";
                            }
                            break;
                        }
                        case (urcl::defines::op_type::comparison): {
                            if (token.type == urcl::token::comparison) break;
                            ++operand;
                        }
                        case (urcl::defines::op_type::imm): {
                            if (!tokenIsImmediate(token, *this) || (!config.useIris && token.original == "_")) {
                                if (config.useUrcx && inst == "IMM" && tokenIsRegister(token, *this)) break;
                                token.parse_error = "Expected immediate value in operand";
                            }
                            break;
                        }
                        case (urcl::defines::op_type::reg): {
                            if (!tokenIsRegister(token, *this)) {
                                token.parse_error = "Expected register in operand";
                            }
                            break;
                        }
                        case (urcl::defines::op_type::basicval): {
                            if (tokenIsRegister(token, *this)) break;
                            if (!config.useBasic) {
                                token.parse_error = "Expected register in operand";
                                break;
                            }
                            if (!tokenIsImmediate(token, *this) || token.original == "_") {
                                token.parse_error = "Expected value in operand";
                            }
                            break;
                        }
                        case (urcl::defines::op_type::val): {
                            if (!tokenIsRegister(token, *this) && !tokenIsImmediate(token, *this) || token.original == "_") {
                                token.parse_error = "Expected register or immediate value in operand";
                            }
                            break;
                        }
                    }
                }

            }

            if (token.type == urcl::token::bracket && token.original == "[") {
                if (inArray) token.parse_error = "Nested array arguments not allowed";
                inArray = true;
                continue;
            }

            if (operand == 1 && inst == "@DEFINE") {
                std::string copy = util::strToUpper(token.original.substr(1));
                if (this->constants.contains(copy)) {
                    token.parse_error = "Constant already defined by implementation";
                }
            }

            if (token.type == urcl::token::literal && inst == "BITS") {
                bits = std::max(bits, (uint16_t)token.value.literal);
            }

            if (!expect) {
                token.parse_error = "Extraneous token after language construct";
            } else {
                switch (token.type) {
                    case (urcl::token::label): {
                        if (!labelDefs.contains(token.original)) {
                            token.parse_error = "Undefined label";
                        } else if (labelDefs.at(token.original).first != currentObjId) {
                            token.parse_error = "Label defined outside of sub-object";
                        }
                        break;
                    }
                    case (urcl::token::symbol): {
                        if (!symbolDefs.contains(token.original)) {
                            token.parse_error = "Undefined symbol";
                        }
                        break;
                    }
                    case (urcl::token::constant): {
                        std::string copy = util::strToUpper(token.original.substr(1));
                        if (this->constants.contains(copy)) break;
                    }
                    case (urcl::token::name): {
                        if (definesDefs.contains(token.original)) break;
                        token.parse_error = "Undefined constant value";
                    }
                    case (urcl::token::escape): {
                        std::string escaped = token.original.substr(token.original.find('\\') + 1);
                        switch (escaped[0]) {
                            case ('\''):
                            case ('"'):
                            case ('\\'):
                            case ('n'):
                            case ('r'):
                            case ('t'):
                            case ('b'):
                            case ('f'):
                            case ('v'):
                            case ('0'): {
                                break;
                            }
                            default: {
                                token.parse_error = "Invalid escape sequence";
                            }
                        }
                    }
                    default:
                        break;
                }
            }

            if (inArray && token.parse_error == "" && !tokenIsImmediate(token, *this) && token.type != urcl::token::string) {
                token.parse_error = "Array contents must be immediate values";
            }
        }
        if (line.size() == 0) continue;
        if (operands && line[0].parse_error == "" && operands->size() > operand && inst != "@DEBUG") {
            line[0].parse_error = "Too few operands in language construct";
        } else if (inArray && line[line.size() - 1].parse_error == "") {
            line[line.size() - 1].parse_error = "Unclosed array argument";
        }
    }
}


std::vector<unsigned int> urcl::source::getTokens() const {
    std::vector<unsigned int> result;
    unsigned int prevLine = 0;
    for (int i = 0; i < code.size(); ++i) {
        const std::vector<urcl::token>& line = code[i];
        unsigned int prevChar = 0;
        int lengthDiff = 0;
        for (const urcl::token& token : line) {
            int tokenType = resolveTokenType(token, *this, this->constants);
            if (tokenType < 0) continue;
            //result.reserve(5);
            result.push_back(i - prevLine);
            result.push_back(token.column - lengthDiff - prevChar);
            result.push_back(util::utf8len(token.original.c_str()));
            result.push_back(tokenType);
            result.push_back(0);
            prevChar = token.column;
            lengthDiff = token.original.length() - util::utf8len(token.original.c_str());
            prevLine = i;
        }
    }
    return result;
}

std::vector<lsp::Diagnostic> urcl::source::getDiagnostics() const {
    std::vector<lsp::Diagnostic> result{};

    for (unsigned int i = 0; i < code.size(); ++i) {
        const std::vector<urcl::token>& line = code[i];
        for (const urcl::token& token : line) {
            if (token.parse_error != "") {
                result.push_back({{{i, token.column}, {i, static_cast<uint>(token.column + token.original.length() )}}, token.parse_error, lsp::DiagnosticSeverity::Error});
            }
            if (token.parse_warning != "") {
                result.push_back({{{i, token.column}, {i, static_cast<uint>(token.column + token.original.length() )}}, token.parse_warning, lsp::DiagnosticSeverity::Warning});
            }
        }
    }
    return result;
}

std::optional<lsp::Location> urcl::source::getDefinitionRange(const lsp::Position& position, const std::filesystem::path& file) const {
    unsigned int row = position.line;
    unsigned int column = position.character;
    int idx = columnToIdx(code[row], column);
    if (idx < 0) return {};
    lsp::Location result;
    const urcl::token& token = code[row][idx];
    switch (token.type) {
        case (urcl::token::label): {
            if (!labelDefs.contains(token.original)) return {};
            urcl::line_number line = labelDefs.at(token.original).second;
            int newToken = iFindNthOperand(code[line], 0);
            if (newToken < 0) return {};
            unsigned int newColumn = idxToColumn(code[line], newToken);
            return {{lsp::FileUri::fromPath(file.string()), {{line, newColumn}, {line, static_cast<uint>(newColumn + util::utf8len(code[line][newToken].original.c_str()))}}}};
        }
        case (urcl::token::constant): {
            std::string copy = util::strToUpper(token.original.substr(1));
            if (constants.contains(copy)) return {};
        }
        case (urcl::token::name): {
            if (!definesDefs.contains(token.original)) return {};
            urcl::line_number line = definesDefs.at(token.original).second;
            std::filesystem::path newFile = definesDefs.at(token.original).first;
            const urcl::source* newSrc;
            if (includes.contains(newFile)) {
                newSrc = &includes.at(newFile);
            } else {
                newSrc = this;
            }
            int newToken = iFindNthOperand(newSrc->code[line], 1);
            if (newToken < 0) return {};
            unsigned int newColumn = idxToColumn(newSrc->code[line], newToken);
            return {{lsp::FileUri::fromPath(newFile.string()), {{line, newColumn}, {line, static_cast<uint>(newColumn + util::utf8len(newSrc->code[line][newToken].original.c_str()))}}}};
        }
        case (urcl::token::symbol): {
            if (!symbolDefs.contains(token.original)) return {};
            urcl::line_number line = symbolDefs.at(token.original).second;
            std::filesystem::path newFile = symbolDefs.at(token.original).first;
            const urcl::source* newSrc;
            if (includes.contains(newFile)) {
                newSrc = &includes.at(newFile);
            } else {
                newSrc = this;
            }
            int newToken = iFindNthOperand(newSrc->code[line], 0);
            if (newToken < 0) return {};
            unsigned int newColumn = idxToColumn(newSrc->code[line], newToken);
            return {{lsp::FileUri::fromPath(newFile.string()), {{line, newColumn}, {line, static_cast<uint>(newColumn + util::utf8len(newSrc->code[line][newToken].original.c_str()))}}}};
        }
        default:
            return {};
    }
}

std::optional<lsp::Range> urcl::source::getTokenRange(const lsp::Position& position) const {
    unsigned int row = position.line;
    unsigned int column = position.character;
    int idx = columnToIdx(code[row], column);
    if (idx < 0) return {};
    lsp::Range result;
    const urcl::token& token = code[row][idx];
    unsigned int newColumn = idxToColumn(code[row], idx);
    return {{{row, newColumn}, {row, static_cast<uint>(newColumn + util::utf8len(token.original.c_str()))}}};
};

std::vector<lsp::FoldingRange> urcl::source::getFoldingRanges() const {
    bool inRange = false;
    unsigned int start;
    std::vector<lsp::FoldingRange> result{};
    for (std::pair<urcl::sub_object, urcl::line_number> delimiter : objectDefs) {
        if (!inRange && delimiter.first == urcl::sub_object::open) {
            inRange = true;
            start = delimiter.second;
        } else if (inRange && delimiter.first == urcl::sub_object::close) {
            inRange = false;
            result.emplace_back(start, delimiter.second);
        }
    }
    return result;
}

int urcl::source::iFindNthOperand(const std::vector<urcl::token>& code, unsigned int operand) {
    int counter = 0;
    for (int i = 0; i < code.size(); ++i) {
        const urcl::token& token = code[i];
        if (token.type == urcl::token::comment) continue;
        if (counter == operand) {
            return i;
        }
        ++counter;
    }
    return -1;
}

const urcl::token *urcl::source::findNthOperand(const std::vector<urcl::token>& code, unsigned int operand) {
    int idx = iFindNthOperand(code, operand);
    if (idx < 0) return nullptr;
    return &code[idx];
}

int urcl::source::columnToIdx(const std::vector<urcl::token>& line, unsigned int column) {
    if (line.size() <= 0) return -1;
    unsigned int col = line[0].column;
    int i;
    for (i = 0; i < line.size(); ++i) {
        col += util::utf8len(line[i].original.c_str());
        if (i < line.size() - 1) {
            col += line[i + 1].column - line[i].column - line[i].original.length(); // whitespace
        }
        if (col > column) break;
    }
    if (i >= line.size()) i = -1;
    return i;
}

unsigned int urcl::source::idxToColumn(const std::vector<urcl::token>& line, unsigned int idx) {
    unsigned int column = line[0].column;
    for (int i = 0; i < idx; ++i) {
        column += util::utf8len(line[i].original.c_str());
        column += line[i + 1].column - line[i].column - line[i].original.length(); // whitespace
    }
    return column;
}

std::vector<urcl::token> urcl::source::parseLine(const std::string& line, bool& inComment, const urcl::config& config) const {
    bool inChar = false;
    bool inStr = false;
    std::string name;
    bool inInst = false;
    bool inName = false;
    bool inConstruct = false;
    std::vector<urcl::token> result{};
    bool otherToken = false;

    bool runHeader = false;
    bool debugMacro = false;
    bool dw = !config.useUir;

    if (inComment) {
        size_t end = line.find("*/");
        if (end == std::string::npos) {
            end = line.length() - 1;
        }
        ++end;
        result.push_back({urcl::token::comment, line.substr(0, end + 1), "", 0, "", "", 0});
    }
    for (uint32_t i = 0; i <= line.size(); ++i) {
        if (inComment) {
            if (line[i] != '*' || line[i + 1] != '/') continue;
            ++i;
            inComment = false;
            continue;
        }
        if ((i == line.size() || util::isWhitespace(line[i])) || (inConstruct && (line[i] == '/' || line[i] == ']' || line[i] == '%'))) {
            if (!inConstruct) continue;
            if (inChar || inStr) {
                name = name + line[i];
                continue;
            }

            result.back().original = name;
            if (inInst) {
                std::transform(name.begin(), name.end(), name.begin(), ::toupper);
                if (name == "DW" || name == "RW") dw = true;
                result.back().strVal = name;
                switch (result.back().type) {
                    case (urcl::token::instruction): {
                        if (runHeader) {
                            if (!urcl::defines::RUN_MODES.contains(name)) {
                                result.back().parse_error = "Unknown RUN header value: " + name;
                                runHeader = false;
                            }
                        } else if (debugMacro) {
                            if (!urcl::defines::DEBUG_MODES.contains(name)) {
                                result.back().parse_error = "Unknown @DEBUG mode: " + name;
                                debugMacro = false;
                            }
                        } else if (!instructions.contains(name)) {
                            result.back().parse_error = "Unknown instruction: " + name;
                        }
                        break;
                    }
                    case (urcl::token::macro): {
                        if (!config.useStandard && !config.useUrcx && !config.useIris) break;
                        if (!macros.contains(name)) {
                            result.back().parse_error = "Unknown macro: " + name;
                        }
                        break;
                    }
                    case (urcl::token::port): {
                        if (!config.useStandard && !config.useUrcx && !config.useIris) break;
                        name.erase(0, 1);
                        if (util::isNumber(name)) {
                            int portNumb = std::stoi(name);
                            if (portNumb > 63 || portNumb < 0) {
                                result.back().parse_error = "Invalid port number: " + name;
                            }
                        } else if (!ports.contains(name)) {
                            result.back().parse_error = "Unknown port: " + name;
                        }
                        break;
                    }
                    default:
                        break;
                }
                if (name == "RUN") {
                    otherToken = false;
                    runHeader = true;
                }
                if (config.useUrcx && name == "@DEBUG") {
                    otherToken = false;
                    debugMacro = true;
                }
                inInst = false;
            } else if (inName) {
                if (config.useIris && name == "_") {
                    result.back().type = urcl::token::literal;
                    result.back().value.literal = 0;
                }
                inName = false;
            } else {
                switch (result.back().type) {
                    case (urcl::token::relative): {
                        if (name[1] != '+' && name[1] != '-') {
                            if (config.useStandard || !config.useUrcx) {
                                result.back().parse_error = "Relative without sign";
                            }
                            name.erase(0, 1);
                        } else {
                            name.erase(0, 2);
                        }
                        if (!util::isNumber(name)) {
                            result.back().parse_error = "Invalid integer in relative";
                        }
                        break;
                    }
                    case (urcl::token::literal): {
                        int base = 10;
                        int sign = 1;
                        if (name[0] == '-') {
                            sign = -1;
                            name.erase(0, 1);
                        } else if (name[0] == '+') {
                            name.erase(0, 1);
                        }

                        if (name.length() > 2 && name[0] == '0') {
                            if (name[1] == 'b') {
                                base = 2;
                                name.erase(0, 2);
                            } else if (name[1] == 'o') {
                                base = 8;
                                name.erase(0, 2);
                            } else if (name[1] == 'x') {
                                base = 16;
                                name.erase(0, 2);
                            }
                        }

                        std::erase(name, '_');

                        bool valid;
                        if (base == 2) {
                            valid = util::isBinary(name);
                        } else if (base == 8) {
                            valid = util::isOctal(name);
                        } else if (base == 16) {
                            valid = util::isHex(name);
                        } else {
                            if (name.find('.') == std::string::npos) {
                                valid = util::isNumber(name);
                            } else {
                                
                                valid = util::isFloat(name);
                                if (valid) {
                                    long double value = std::stold(name);
                                    result.back().value.real = value * sign;
                                    result.back().type = urcl::token::real;
                                    if (config.useStandard && !config.useIris) {
                                        result.back().parse_error = "Floats are non-standard";
                                    }
                                } else {
                                    result.back().parse_error = "Invalid float literal";
                                    result.back().value.real = 0;
                                    result.back().type = urcl::token::name;
                                }
                                break;
                            }
                        }
                        if (valid) {
                            int64_t value;
                            try {
                                value = std::stoll(name, nullptr, base);
                            } catch (std::exception e) {
                                value = 0;
                            }
                            result.back().value.literal = value * sign;
                        } else {
                            result.back().parse_error = "Invalid integer literal";
                            result.back().value.literal = 0;
                            result.back().type = urcl::token::name;
                        }
                        break;
                    }
                    case (urcl::token::mem):
                    case (urcl::token::reg): {

                        if (name.starts_with("[M") || name.starts_with("[m") || name.starts_with("[#")) {
                            if (!util::isNumber(name.substr(2))) {
                                result.back().parse_error = "Invalid integer in memory address";
                                result.back().type = urcl::token::name;
                            } else if (line[i] != ']') {
                                result.back().parse_error = "Unterminated uir memory reference";
                                result.back().type = urcl::token::name;
                            }
                            if (line[i] == ']') {
                                result.back().original += ']';
                                ++i;
                            }
                            break;
                        }

                        if (!util::isNumber(name.substr(1))) {
                            result.back().parse_error = result.back().type == urcl::token::reg ? "Invalid integer in register" : "Invalid integer in memory address";
                            result.back().type = urcl::token::name;
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
            inConstruct = false;
            if (!(i < line.length() && (line[i] == '/' || line[i] == ']' || line[i] == '%'))) continue;
        }

        if (inStr && line[i] == '\\') {
            result.back().original = name;
            result.push_back({urcl::token::escape, line.substr(i, 2), "", 0, "", "", i});
            ++i;
            result.push_back({urcl::token::string, "", "", 0, "", "", i + 1});
            name = "";
            continue;
        }

        if (inChar && line[i] == '\\') {
            name = name + line[i] + line[i + 1];
            result.back().type = urcl::token::escape;
            ++i;
            continue;
        }

        if ((inStr && line[i] == '"') || (inChar && line[i] == '\'')) {
            name = name + line[i];
            result.back().original = name;
            inStr = false;
            inChar = false;
            inConstruct = false;
            continue;
        }

        const urcl::token& current = result.back();
        if (inConstruct && !std::isdigit(line[i]) && (current.type == token::reg || current.type == token::mem)) {
            result.back().type = urcl::token::name;
            inName = true;
        }

        if (inConstruct) {
            name = name + line[i];
            continue;
        }

        if (line[i] == '/' && line[i + 1] == '/') {
            result.push_back({urcl::token::comment, line.substr(i), "", 0, "", "", i});
            break;
        }

        if (line[i] == '/' && line[i + 1] == '*') {
            size_t end = line.find("*/", i + 2);
            if (end == std::string::npos) {
                end = line.length() - 1;
            }
            ++end;
            result.push_back({urcl::token::comment, line.substr(i, end - i + 1), "", 0, "", "", i});
            inComment = true;
            ++i;
            continue;
        }

        if (line[i] == '.') {
            result.push_back({urcl::token::label, "", "", 0, "", "", i});
            name = line[i];
            inConstruct = true;
            otherToken = true;
            continue;
        } else if (line[i] == '!') {
            result.push_back({urcl::token::symbol, "", "", 0, "", "", i});
            name = line[i];
            inConstruct = true;
            otherToken = true;
            continue;
        }

        if (!otherToken) {
            if (line[i] == '@') {
                result.push_back({urcl::token::macro, "", "", 0, "", "", i});
            } else {
                result.push_back({urcl::token::instruction, "", "", 0, "", "", i});
            }
            name = line[i];
            inConstruct = true;
            inInst = true;
            otherToken = true;
            continue;
        }

        otherToken = true;

        inConstruct = true;

        name = line[i];
        if (line[i] == 'R' || line[i] == 'r' || line[i] == '$') {
            result.push_back({urcl::token::reg, "", "", 0, "", "", i});
        } else if ((line[i] == 'S' || line[i] == 's') && (line[i + 1] == 'P' || line[i + 1] == 'p')) {
            result.push_back({urcl::token::reg, line.substr(i, 2), "", -1, "", "", i});
            inConstruct = false;
            ++i;
        } else if ((line[i] == 'P' || line[i] == 'p') && (line[i + 1] == 'C' || line[i + 1] == 'c')) {
            result.push_back({urcl::token::reg, line.substr(i, 2), "", -2, "", "", i});
            inConstruct = false;
            ++i;
        } else if (line[i] == 'M' || line[i] == 'm' || line[i] == '#') {
            result.push_back({urcl::token::mem, "", "", 0, "", "", i});
        } else if (std::isdigit(line[i]) || line[i] == '+' || line[i] == '-') {
            result.push_back({urcl::token::literal, "", "", 0, "", "", i});
        } else if (line[i] == '%') {
            result.push_back({urcl::token::port, "", "", 0, "", "", i});
            inInst = true;
        } else if (line[i] == '~') {
            result.push_back({urcl::token::relative, "", "", 0, "", "", i});
        } else if (!dw && line[i] == '[' && (line[i + 1] == 'M' || line[i + 1] == 'm' || line[i + 1] == '#')) {
            result.push_back({urcl::token::reg, "", "", 0, "", "", i});
            name = name + line[i + 1];
            ++i;
        } else if (line[i] == '[' || line[i] == ']') {
            result.push_back({urcl::token::bracket, line.substr(i, 1), "", 0, "", "", i});
            inConstruct = false;
        } else if (line[i] == '@') {
            result.push_back({urcl::token::constant, "", "", 0, "", "", i});
        } else if (line[i] == '"') {
            result.push_back({urcl::token::string, "", "", 0, "", "", i});
            inStr = true;
        } else if (line[i] == '\'') {
            result.push_back({urcl::token::character, "", "", 0, "", "", i});
            inChar = true;
        } else if ((line[i] == '>' || line[i] == '<' || line[i] == '=') && line[i + 1] == '=') {
            result.push_back({urcl::token::comparison, line.substr(i, 2), "", 0, "", "", i});
            inConstruct = false;
            ++i;
        } else {
            result.push_back({urcl::token::name, "", "", 0, "", "", i});
            inName = true;
        }
    }
    if (inStr) {
        result.back().original = name;
        result.back().parse_error = "Unterminated string";
    }
    if (inChar) {
        result.back().original = name;
        result.back().parse_error = "Unterminated character";
    }
    return result;
}

int urcl::source::resolveTokenType(const urcl::token& token, const urcl::source& original, const std::unordered_set<std::string>& constants) const {
    int tokenType;
    switch (token.type) {
        case (urcl::token::instruction):
            tokenType = 0;
            break;
        case (urcl::token::reg):
            tokenType = 1;
            break;
        case (urcl::token::relative):
        case (urcl::token::literal):
        case (urcl::token::real):
            tokenType = 2;
            break;
        case (urcl::token::label):
            tokenType = 3;
            break;
        case (urcl::token::comment):
            tokenType = 4;
            break;
        case (urcl::token::port):
        case (urcl::token::mem):
            tokenType = 5;
            break;
        case (urcl::token::macro):
            tokenType = 7;
            break;
        case (urcl::token::character):
        case (urcl::token::string):
            tokenType = 8;
            break;
        case (urcl::token::escape):
            tokenType = 9;
            break;
        case (urcl::token::comparison):
        case (urcl::token::bracket):
            tokenType = 10;
            break;
        case (urcl::token::symbol):
            if (token.original.length() >= 2 && token.original.substr(0, 2) == "!!") {
                tokenType = 11;
            } else {
                tokenType = 3;
            }
            break;
        case (urcl::token::constant): {
            std::string copy = util::strToUpper(token.original.substr(1));
            if (constants.contains(copy)) {
                tokenType = 7;
                break;
            }
        }
        case (urcl::token::name): {
            if (original.definesDefs.contains(token.original)) {
                const std::filesystem::path& newPath = original.definesDefs.at(token.original).first;
                const urcl::source *newSrc;
                if (original.includes.contains(newPath)) {
                    newSrc = &original.includes.at(newPath);
                } else {
                    newSrc = &original;
                }
                const urcl::token *newToken = urcl::source::findNthOperand(newSrc->code[original.definesDefs.at(token.original).second], 2);
                if (newToken == nullptr) {
                    tokenType = -1;
                    return tokenType;
                }
                tokenType = newSrc->resolveTokenType(*newToken, original, constants);
                if (tokenType == 9) tokenType = 8;
                break;
            } else {
                tokenType = -1;
            }
            break;
        }
        default:
            tokenType = -1;
    }
    return tokenType;
}

std::vector<lsp::CompletionItem> urcl::source::getCompletion(const lsp::Position& position, const urcl::config& config) const {
    unsigned int row = position.line;
    unsigned int column = position.character - 1;
    int idx = columnToIdx(code[row], column);
    std::vector<lsp::CompletionItem> result;
    if (idx < 0) return result;
    const urcl::token& token = code[row][idx];
    switch (token.type) {
        case (urcl::token::label): {
            int opIdx;
            for (opIdx = 0; opIdx < idx; ++opIdx) {
                if (code[row][opIdx].type == urcl::token::comment) continue;
                break;
            }
            if (opIdx == idx) break; // No prior operand exists, thus it is a definition;

            // Iterate over source to find the current subobject id
            urcl::object_id nextObjId = 1;
            urcl::object_id currentObjId = 0;
            for (int i = 0; i <= row; ++i) {
                const std::vector<urcl::token>& line = this->code[i];
                for (const urcl::token& token : line) {
                    if (token.type == urcl::token::comment) continue;
                    if (token.type == urcl::token::symbol) {
                        if (currentObjId == 0 && token.original.length() >= 3 && token.original.starts_with("!!!")) {
                            currentObjId = nextObjId++;
                        } else if (currentObjId != 0 && token.original.length() >= 2 && token.original.starts_with("!!")) {
                            currentObjId = 0;
                        }
                    }
                    break;
                }
            }

            for (std::pair<std::string, std::pair<urcl::object_id, urcl::line_number>> label : labelDefs) {
                if (label.second.first == currentObjId && label.first.starts_with(token.original)) {
                    result.emplace_back(label.first.substr(1));
                }
            }
            break;
        }
        case (urcl::token::symbol): {
            int opIdx;
            for (opIdx = 0; opIdx < idx; ++opIdx) {
                if (code[row][opIdx].type == urcl::token::comment) continue;
                break;
            }
            if (opIdx == idx) break; // No prior operand exists, thus it is a definition;

            for (std::pair<std::string, std::pair<std::filesystem::path, urcl::line_number>> symbol : symbolDefs) {
                if (symbol.first.starts_with("!!")) continue;
                if (symbol.first.starts_with(token.original)) {
                    result.emplace_back(symbol.first.substr(1));
                }
            }
            break;
        }
        case (urcl::token::name):
        case (urcl::token::constant): {
            if (token.original[0] == '@') {
                for (const std::string& constant : constants) {
                    if (constant.starts_with(token.original.substr(1))) {
                        if (config.useLowercase) {
                            result.emplace_back(util::strToLower(constant));
                        } else {
                            result.emplace_back(constant);
                        }
                    }
                }
            }
            for (const std::pair<std::string, std::pair<std::filesystem::path, urcl::line_number>>& def : definesDefs) {
                if (def.first.starts_with(token.original)) {
                    if (token.original.at(0) == '@') {
                        result.emplace_back(def.first.substr(1));
                    } else {
                        result.emplace_back(def.first);
                    }
                }
            }
            break;
        }
        case (urcl::token::instruction): {
            do {
                --idx;
            } while (idx > 0 && code[row][idx].type != urcl::token::macro);
            if (code[row][idx].type == urcl::token::macro) {
                for (const std::string& mode : urcl::defines::DEBUG_MODES) {
                    if (mode.starts_with(token.strVal)) {
                        if (config.useLowercase) {
                            result.emplace_back(util::strToLower(mode));
                        } else {
                            result.emplace_back(mode);
                        }
                    }
                }
                break;
            }
            for (const std::string& inst : instructions) {
                if (inst.starts_with(token.strVal)) {
                    if (config.useLowercase) {
                        result.emplace_back(util::strToLower(inst));
                    } else {
                        result.emplace_back(inst);
                    }
                }
            }
            break;
        }
        case (urcl::token::macro): {
            for (const std::string& macro : macros) {
                if (macro.starts_with(token.strVal)) {
                    if (config.useLowercase) {
                        result.emplace_back(util::strToLower(macro.substr(1)));
                    } else {
                        result.emplace_back(macro.substr(1));
                    }
                }
            }
            break;
        }
        case (urcl::token::port): {
            for (const std::string& port : ports) {
                if (port.starts_with(token.strVal.substr(1))) {
                    if (config.useLowercase) {
                        result.emplace_back(util::strToLower(port));
                    } else {
                        result.emplace_back(port);
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    if (result.empty()) result.emplace_back("");
    return result;
}

std::optional<std::string> urcl::source::getHover(const lsp::Position& position, const urcl::config& config) const {
    unsigned int row = position.line;
    unsigned int column = position.character;
    int idx = columnToIdx(code[row], column);
    if (idx < 0) return {};
    const urcl::token& token = code[row][idx];
    return urcl::source::getHover(token, config);
}

std::optional<std::string> urcl::source::getHover(const urcl::token& token, const urcl::config& config) const {
    switch (token.type) {
        case (urcl::token::macro):
        case (urcl::token::instruction): {
            if (!urcl::defines::INST_INFO.contains(token.strVal)) return {};
            std::pair<urcl::defines::description, std::vector<urcl::defines::op_type>> info = urcl::defines::INST_INFO.at(token.strVal);
            return {info.first};
        }
        case (urcl::token::port): {
            std::string result = "";
            if (urcl::defines::IN_INFO.contains(token.strVal)) {
                result = "IN" + token.strVal + ": " + urcl::defines::IN_INFO.at(token.strVal).first + "\\\n";
            } else if (urcl::defines::STD_PORTS.contains(token.strVal.substr(1))) {
                result = "IN" + token.strVal + ": Implementation defined functionality\\\n";
            }
            if (urcl::defines::OUT_INFO.contains(token.strVal)) {
                result += "OUT" + token.strVal + ": " + urcl::defines::OUT_INFO.at(token.strVal).first + "\\\n";
            } else if (urcl::defines::STD_PORTS.contains(token.strVal.substr(1))) {
                result += "OUT" + token.strVal + ": Implementation defined functionality\\\n";
            }
            if (urcl::defines::IRIS_PORTS.contains(token.strVal.substr(1))) {
                result += "IRIS port";
            }
            if (urcl::defines::URCX_PORTS.contains(token.strVal.substr(1))) {
                result += "URCX port";
            }
            if (urcl::defines::PORT_NUMBS.contains(token.strVal)) {
                result += "Equivalent to %" + std::to_string(urcl::defines::PORT_NUMBS.at(token.strVal));
            }
            return result;
        }
        case (urcl::token::literal): {
            int64_t numb = token.value.literal;
            return util::intHover(numb, bits, config.useIris);
        }
        case (urcl::token::real): {
            if (bits == 16 && config.useIris) {
                return util::intHover(util::floatToIris(token.value.real), bits, config.useIris);
            } else if (bits == 32 && sizeof(float) == sizeof(uint32_t)) {
                float real = token.value.real;
                return util::intHover(reinterpret_cast<uint32_t &>(real), bits, config.useIris);
            } else if (bits == 64 && sizeof(double) == sizeof(int64_t)) {
                double real = token.value.real;
                return util::intHover(reinterpret_cast<int64_t &>(real), bits, config.useIris);
            }
        }
        case (urcl::token::character): {
            std::string character = token.original.substr(1);
            int64_t numb = util::from_utf8(character);
            return util::intHover(numb, bits, config.useIris);
        }
        case (urcl::token::escape): {
            std::string escaped = token.original.substr(token.original.find('\\') + 1);
            int64_t numb;
            switch (escaped[0]) {
                case ('\''):
                    numb = '\'';
                    break;
                case ('"'):
                    numb = '"';
                    break;
                case ('\\'):
                    numb = '\\';
                    break;
                case ('n'):
                    numb = '\n';
                    break;
                case ('r'):
                    numb = '\r';
                    break;
                case ('t'):
                    numb = '\t';
                    break;
                case ('b'):
                    numb = '\b';
                    break;
                case ('f'):
                    numb = '\f';
                    break;
                case ('v'):
                    numb = '\v';
                    break;
                case ('0'): {
                    numb = '\0';
                    break;
                }
                default: {
                    return {};
                }
            }
            return util::intHover(numb, bits, config.useIris);
        }
        case (urcl::token::constant): {
            std::string copy = util::strToUpper(token.original.substr(1));
            if (constants.contains(copy)) break;
        }
        case (urcl::token::name): {
            if (definesDefs.contains(token.original)) {
                const std::filesystem::path& newPath = definesDefs.at(token.original).first;
                const urcl::source *newSrc;
                if (includes.contains(newPath)) {
                    newSrc = &includes.at(newPath);
                } else {
                    newSrc = this;
                }
                const urcl::token *newToken = urcl::source::findNthOperand(newSrc->code[definesDefs.at(token.original).second], 2);
                if (newToken == nullptr) {
                    break;
                }
                return getHover(*newToken, config);
            }

        }
        default: {
            break;
        }
    }
    return {};
}

std::vector<lsp::Location> urcl::source::getReferences(const lsp::Position& position, const lsp::DocumentUri& uri) const {
    std::vector<lsp::Location> result;
    unsigned int row = position.line;
    unsigned int column = position.character;
    int idx = columnToIdx(code[row], column);
    if (idx < 0) return result;
    const urcl::token& token = code[row][idx];
    if (token.type == urcl::token::constant) {
        std::string copy = util::strToUpper(token.original.substr(1));
        if (constants.contains(copy)) return result;
    } else if (token.type != urcl::token::label && token.type != urcl::token::symbol && token.type != urcl::token::name) {
        return result;
    }
    
    unsigned int length = util::utf8len(token.original.c_str());
    for (unsigned int j = 0; j < code.size(); ++j) {
        if (j == position.line) continue;
        const std::vector<urcl::token>& line = code[j];
        for (unsigned int i = 0; i < line.size(); ++i) {
            const urcl::token& token2 = line[i];
            if (token.type == token2.type && token.original == token2.original) {
                unsigned int column = idxToColumn(line, i);
                if (column == position.character) continue;
                lsp::Location loc = lsp::Location{uri, {{j, column}, {j, (column + length)}}};
                result.push_back(std::move(loc));
            }
        }
    }
    if (token.type == urcl::token::label) return result;

    for (const std::pair<std::filesystem::path, urcl::source>& include : includes) {
        lsp::DocumentUri newUri = lsp::FileUri::fromPath(include.first.string());
        const urcl::source& included = include.second;
        for (unsigned int j = 0; j < included.code.size(); ++j) {
            const std::vector<urcl::token>& line = included.code[j];
            for (unsigned int i = 0; i < line.size(); ++i) {
                const urcl::token& token2 = line[i];
                if (token.type == token2.type && token.original == token2.original) {
                    unsigned int column = idxToColumn(line, i);
                    lsp::Location loc = lsp::Location{newUri, {{j, column}, {j, (column + length)}}};
                    result.push_back(std::move(loc));
                }
            }
        }
    }

    return result;
}

bool urcl::source::tokenIsRegister(const urcl::token& token, const urcl::source& original) const {
    switch (token.type) {
        case (urcl::token::reg): {
            return true;
        }
        case (urcl::token::constant): {
            std::string copy = util::strToUpper(token.original.substr(1));
            if (constants.contains(copy)) return false;
        }
        case (urcl::token::name): {
            if (original.definesDefs.contains(token.original)) {
                const std::filesystem::path& newPath = original.definesDefs.at(token.original).first;
                const urcl::source *newSrc;
                if (original.includes.contains(newPath)) {
                    newSrc = &original.includes.at(newPath);
                } else {
                    newSrc = &original;
                }
                const urcl::token *newToken = urcl::source::findNthOperand(newSrc->code[original.definesDefs.at(token.original).second], 2);
                if (newToken == nullptr) {
                    return false;
                }
                return newSrc->tokenIsRegister(*newToken, original);
            }
            return false;
        }
        default: {
            return false;
        }
    }
}

bool urcl::source::tokenIsImmediate(const urcl::token& token, const urcl::source& original) const {
    switch (token.type) {
        case (urcl::token::character):
        case (urcl::token::escape):
        case (urcl::token::label):
        case (urcl::token::literal):
        case (urcl::token::mem):
        case (urcl::token::real):
        case (urcl::token::relative):
        case (urcl::token::symbol): {
            return true;
        }
        case (urcl::token::constant): {
            std::string copy = util::strToUpper(token.original.substr(1));
            if (constants.contains(copy)) return true;
        }
        case (urcl::token::name): {
            if (original.definesDefs.contains(token.original)) {
                const std::filesystem::path& newPath = original.definesDefs.at(token.original).first;
                const urcl::source *newSrc;
                if (original.includes.contains(newPath)) {
                    newSrc = &original.includes.at(newPath);
                } else {
                    newSrc = &original;
                }
                const urcl::token *newToken = urcl::source::findNthOperand(newSrc->code[original.definesDefs.at(token.original).second], 2);
                if (newToken == nullptr) {
                    return false;
                }
                return newSrc->tokenIsImmediate(*newToken, original);
            }
            return false;
        }
        default: {
            return false;
        }
    }
}