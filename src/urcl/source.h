#ifndef SOURCE_H
#define SOURCE_H

#include "config.h"
#include "token.h"

#include <vector>
#include <map>
#include <unordered_set>
#include <filesystem>
#include <lsp/types.h>

namespace urcl {
    using line_number = unsigned int;
    using object_id = unsigned int;

    enum sub_object {
        open,
        close
    };

    class source {
        public:
            source();
            source(const std::vector<std::string>& source, const urcl::config& config);

            void updateReferences(const std::unordered_map<std::filesystem::path, source>& all, const urcl::config& config);
            void updateDefinitions(const std::filesystem::path& loc, const urcl::config& config);
            void updateErrors(const urcl::config& config);

            std::vector<unsigned int> getTokens() const;
            std::vector<lsp::Diagnostic> getDiagnostics() const;
            std::optional<lsp::Location> getDefinitionRange(const lsp::Position& position, const std::filesystem::path& file) const;
            std::optional<lsp::Range> getTokenRange(const lsp::Position& position) const;
            std::vector<lsp::FoldingRange> getFoldingRanges() const;
            std::vector<lsp::CompletionItem> getCompletion(const lsp::Position& position, const urcl::config& config) const;
            std::optional<std::string> getHover(const lsp::Position& position, const urcl::config& config) const;
            std::vector<lsp::Location> getReferences(const lsp::Position& position, const lsp::DocumentUri& uri) const;
        private:
            std::optional<std::string> getHover(const urcl::token& token, const urcl::config& config) const;
            std::vector<std::vector<token>> code;
            std::unordered_map<std::string, std::pair<urcl::object_id, urcl::line_number>> labelDefs;
            std::unordered_map<std::string, std::pair<std::filesystem::path, urcl::line_number>> definesDefs;
            std::unordered_map<std::string, std::pair<std::filesystem::path, urcl::line_number>> symbolDefs;
            std::vector<std::pair<urcl::sub_object, urcl::line_number>> objectDefs;
            std::unordered_map<std::filesystem::path, source> includes;
            std::unordered_set<std::string> constants;

            std::unordered_set<std::string> instructions{};
            std::unordered_set<std::string> macros{};
            std::unordered_set<std::string> ports{};

            uint16_t bits = 8;

            static int iFindNthOperand(const std::vector<urcl::token>& code, unsigned int operand);
            static const token *findNthOperand(const std::vector<token>& code, unsigned int operand);
            static int columnToIdx(const std::vector<urcl::token>& line, unsigned int column);
            static unsigned int idxToColumn(const std::vector<urcl::token>& line, unsigned int idx);
            
            bool tokenIsImmediate(const urcl::token& token, const urcl::source& original) const;
            bool tokenIsRegister(const urcl::token& token, const urcl::source& original) const;
            bool tokenIsR0(const urcl::token& token, const urcl::source& original) const;

            std::vector<token> parseLine(const std::string& line, bool& inComment, const urcl::config& config) const;
            int resolveTokenType(const urcl::token& token, const urcl::source& original, const std::unordered_set<std::string>& constants) const;

            void updateDefinitions(urcl::source& code, const std::filesystem::path& loc, bool base);
    };
}


#endif