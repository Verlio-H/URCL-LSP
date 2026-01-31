#include <lsp/messages.h>
#include <lsp/connection.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>
#include <lsp/types.h>

#include <stdio.h>
#include <string.h>
#include <string_view>
#include <ranges>

#include "urcl/source.h"
#include "urcl/config.h"
#include "util.h"

typedef unsigned int uint;

std::vector<std::string> splitString(std::string &str) {
    std::vector<std::string> result{};
    std::stringstream stream(str);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.ends_with("\r")) line.erase(line.end());
        result.emplace_back(line);
    }
    if (str.ends_with("\n")) {
        result.emplace_back("");
    }
    return result;
}

std::vector<std::string> replaceRange(const std::vector<std::string> &str, lsp::Range range, const std::vector<std::string> &newVal) {
    std::vector<std::string> result(str.begin(), str.begin() + range.start.line);
    std::string_view start_line = str.at(range.start.line);
    std::string_view end_line = str.at(range.end.line);
    if (newVal.size() <= 1) {
        std::string_view insertion;
        if (newVal.size() == 0) {
            insertion = "";
        } else {
            insertion = newVal.at(0);
        }
        std::string_view start = start_line.substr(0, util::utf16index(start_line, range.start.character));
        std::string_view end = end_line.substr(util::utf16index(end_line, range.end.character));
        auto joined = std::array{start, insertion, end} | std::views::join;
        result.emplace_back(joined.begin(), joined.end());
    } else {
        std::string_view first_part = start_line.substr(0, util::utf16index(start_line, range.start.character));
        auto first_joined = std::array{first_part, (std::string_view)newVal.at(0)} | std::views::join;
        result.emplace_back(first_joined.begin(), first_joined.end());
        result.insert(result.end(), newVal.begin() + 1, newVal.end() - 1);
        std::string_view final_line = *(newVal.end() - 1);
        std::string_view last_part = end_line.substr(util::utf16index(end_line, range.end.character));
        auto last_joined = std::array{final_line, last_part} | std::views::join;
        result.emplace_back(last_joined.begin(), last_joined.end());
    }
    result.insert(result.end(), str.begin() + range.end.line + 1, str.end());
    return result;
}

int main(int argc, char *argv[]) {
    lsp::Connection connection = lsp::Connection(lsp::io::standardIO());
    lsp::MessageHandler messageHandler = lsp::MessageHandler(connection);
    std::unordered_map<std::filesystem::path, urcl::source> code;
    std::unordered_map<std::filesystem::path, urcl::config> config;
    std::unordered_map<std::filesystem::path, std::vector<std::string>> documents;

    const char *escaped = "escape";
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--no-escape")) {
            escaped = "string";
        }
    }

    messageHandler.add<lsp::requests::Initialize>(
        [escaped](lsp::requests::Initialize::Params&& params) {
            return lsp::requests::Initialize::Result{
                .capabilities = {
                    .positionEncoding = lsp::PositionEncodingKind::UTF16,
                    .textDocumentSync = lsp::TextDocumentSyncOptions(true, lsp::TextDocumentSyncKind::Incremental, false, false, true),
                    .completionProvider = lsp::CompletionOptions{true, {{".", "@", "!", "%"}}},
                    .hoverProvider = true,
                    .definitionProvider = true,
                    .referencesProvider = true,
                    .foldingRangeProvider = true,
                    .semanticTokensProvider = lsp::SemanticTokensOptions(false, {{"keyword", "variable", "number", "function", "comment", "class", "operator", "macro", "string", escaped, "operator", "namespace"}, {}}, false, true)
                },
                .serverInfo = lsp::InitializeResultServerInfo{
                    .name    = "URCL Language Server",
                    .version = "1.0.0"
                }
            };
        }
    ).add<lsp::notifications::TextDocument_DidOpen>(
        [&code, &config, &documents](lsp::notifications::TextDocument_DidOpen::Params&& params) {
            std::vector<std::string> document = splitString(params.textDocument.text);
            std::filesystem::path str = params.textDocument.uri.path();
            config.emplace(str, str);
            code.emplace(str, urcl::source(document, config.at(str)));
            code[str].updateReferences(code, config[str]);
            code[str].updateDefinitions(str, config[str]);
            code[str].updateErrors(config[str]);
            documents[str] = std::move(document);
        }
    ).add<lsp::notifications::TextDocument_DidClose>(
        [&code, &config, &documents](lsp::notifications::TextDocument_DidClose::Params&& params) {
            std::filesystem::path str = params.textDocument.uri.path();
            code.erase(str);
            config.erase(str);
            documents.erase(str);
        }
    ).add<lsp::notifications::TextDocument_DidSave>(
        [&code, &config, &documents, &messageHandler](lsp::notifications::TextDocument_DidSave::Params&& params) {
            std::filesystem::path str = params.textDocument.uri.path();
            config[str] = str;

            code[str] = urcl::source(documents[str], config[str]);
            code[str].updateReferences(code, config[str]);
            code[str].updateDefinitions(str, config[str]);
            code[str].updateErrors(config[str]);
            lsp::notifications::TextDocument_PublishDiagnostics::Params errorParam{params.textDocument.uri, code[str].getDiagnostics()};
            messageHandler.sendNotification<lsp::notifications::TextDocument_PublishDiagnostics>(std::move(errorParam));
        }
    ).add<lsp::notifications::TextDocument_DidChange>(
        [&code, &config, &documents](lsp::notifications::TextDocument_DidChange::Params&& params) {
            for (lsp::TextDocumentContentChangeEvent change : params.contentChanges) {
                std::filesystem::path str = params.textDocument.uri.path();
                if (std::holds_alternative<lsp::TextDocumentContentChangeEvent_Text>(change)) {
                    lsp::TextDocumentContentChangeEvent_Text fullChange = std::get<lsp::TextDocumentContentChangeEvent_Text>(change);
                    std::vector<std::string> document = splitString(fullChange.text);
                    code[str] = urcl::source(document, config[str]);
                    code[str].updateReferences(code, config[str]);
                    code[str].updateDefinitions(str, config[str]);
                    code[str].updateErrors(config[str]);
                    documents[str] = std::move(document);
                } else {
                    lsp::TextDocumentContentChangeEvent_Range_Text rangeChange = std::get<lsp::TextDocumentContentChangeEvent_Range_Text>(change);
                    std::vector<std::string> newContents = splitString(rangeChange.text);
                    documents[str] = replaceRange(documents[str], rangeChange.range, newContents);
                    code[str] = urcl::source(documents[str], config[str]);
                    code[str].updateReferences(code, config[str]);
                    code[str].updateDefinitions(str, config[str]);
                    code[str].updateErrors(config[str]);
                }
            }
        }
    ).add<lsp::requests::TextDocument_SemanticTokens_Full>(
        [&code, &config, &messageHandler](lsp::requests::TextDocument_SemanticTokens_Full::Params&& params) {
            std::filesystem::path str = params.textDocument.uri.path();
            std::vector<uint> tokens = code[str].getTokens();

            lsp::notifications::TextDocument_PublishDiagnostics::Params errorParam{params.textDocument.uri, code[str].getDiagnostics()};
            messageHandler.sendNotification<lsp::notifications::TextDocument_PublishDiagnostics>(std::move(errorParam));
            return lsp::requests::TextDocument_SemanticTokens_Full::Result {{tokens}};
        }
    ).add<lsp::requests::TextDocument_Definition>(
        [&code](lsp::requests::TextDocument_Definition::Params&& params) {
            std::filesystem::path str = params.textDocument.uri.path();
            lsp::requests::TextDocument_Definition::Result result;
            std::optional<lsp::Location> loc = code[str].getDefinitionRange(params.position, str);
            if (!loc.has_value()) {
                result = nullptr;
                return result;
            }
            std::optional<lsp::Range> sourceRange = code[str].getTokenRange(params.position);
            if (!sourceRange.has_value()) {
                result = {loc.value()};
                return result;
            }
            result.emplace({(std::vector<lsp::LocationLink>){(lsp::LocationLink){loc->uri, loc->range, loc->range, {sourceRange}}}});
            return result;
        }
    ).add<lsp::requests::TextDocument_FoldingRange>(
        [&code](lsp::requests::TextDocument_FoldingRange::Params&& params) {
            std::filesystem::path str = params.textDocument.uri.path();
            return lsp::requests::TextDocument_FoldingRange::Result{code[str].getFoldingRanges()};
        }
    ).add<lsp::requests::TextDocument_Completion>(
        [&code, &config](lsp::requests::TextDocument_Completion::Params&& params) {
            std::filesystem::path str = params.textDocument.uri.path();

            std::vector<lsp::CompletionItem> result = code[str].getCompletion(params.position, config[str]);
            return lsp::requests::TextDocument_Completion::Result{result};
        }
    ).add<lsp::requests::TextDocument_Hover>(
        [&code, &config](lsp::requests::TextDocument_Hover::Params&& params) {
            std::filesystem::path str = params.textDocument.uri.path();
            std::optional<std::string> hover = code[str].getHover(params.position, config[str]);
            if (!hover.has_value()) return lsp::requests::TextDocument_Hover::Result{};
            return lsp::requests::TextDocument_Hover::Result{{hover->data(), code[str].getTokenRange(params.position)}};
        }
    ).add<lsp::requests::TextDocument_References>(
        [&code](lsp::requests::TextDocument_References::Params&& params) {
            std::filesystem::path str = params.textDocument.uri.path();
            return lsp::requests::TextDocument_References::Result{code[str].getReferences(params.position, params.textDocument.uri)};
        }
    );
    
    while (true) {
        messageHandler.processIncomingMessages();
    }

    return 0;
}
