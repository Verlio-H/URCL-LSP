#include "source.h"
#include "../util.h"
#include "defines.h"

#include <fstream>

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
        int operand = -1;
        std::string_view inst;
        for (urcl::token& token : line) {
            if (token.type == urcl::token::comment) continue;
            ++operand;
            if (operand == 0 && (token.type == urcl::token::label || token.type == urcl::token::symbol)) {
                if (token.original.length() >= 3 && token.original.substr(0, 3) == "!!!") {
                    currentObjId = nextObjId++;
                } else if (token.original.length() >= 2 && token.original.substr(0, 2) == "!!") {
                    currentObjId = 0;
                }
                expect = false;
                continue;
            }
            if (token.type == urcl::token::instruction || token.type == urcl::token::macro) {
                inst = std::string_view(token.strVal);
            }
            if (token.parse_error != "") continue;

            if (operand == 1 && inst == "@DEFINE") {
                std::string copy = util::strToUpper(token.original.substr(1));
                if (this->constants.contains(copy)) {
                    token.parse_error = "Constant already defined by implementation";
                }
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
                    default:
                        break;
                }
            }
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

std::optional<lsp::Location> urcl::source::getDefinitionRange(const lsp::Position& position, std::filesystem::path file) const {
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
    unsigned int col = 0;
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
    unsigned int column = 0;
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
                        if (util::isNumber(name)) {
                            int32_t offset = std::stoi(name);
                            result.back().value.relative = offset;
                        } else {
                            result.back().parse_error = "Invalid integer in relative";
                            result.back().value.relative = 0;
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
                            int64_t value = std::stoi(name, nullptr, base);
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

                        name.erase(0, 1);
                        if (util::isNumber(name)) {
                            uint32_t value = std::stoi(name);
                            result.back().value.reg = value;
                        } else {
                            result.back().parse_error = result.back().type == urcl::token::reg ? "Invalid integer in register" : "Invalid integer in memory address";
                            result.back().value.reg = 0;
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
        name = line[i];
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
    switch (token.type) {
        case (urcl::token::macro):
        case (urcl::token::instruction): {
            if (!urcl::defines::INST_INFO.contains(token.strVal)) return {};
            std::pair<urcl::defines::description, std::vector<urcl::defines::op_type>> info = urcl::defines::INST_INFO.at(token.strVal);
            return {info.first};
        }
        default: {
            return {};
        }
    }
}