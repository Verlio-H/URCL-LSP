#include <lsp/messages.h>
#include <lsp/connection.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>
#include <lsp/types.h>

#include <stdio.h>
#include <string.h>

#include "urcl/source.h"
#include "urcl/config.h"

typedef unsigned int uint;

std::vector<std::string> splitString(std::string &str) {
    std::vector<std::string> result{};
    std::stringstream stream(str);
    std::string line;

    bool inComment = false;
    while (std::getline(stream, line, '\n')) {
        result.emplace_back(line);
    }
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
                    .textDocumentSync = lsp::TextDocumentSyncOptions(true, lsp::TextDocumentSyncKind::Full, true),
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
            code[str].updateDefinitions(str);
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
    ).add<lsp::notifications::TextDocument_WillSave>(
        [&code, &config, &documents, &messageHandler](lsp::notifications::TextDocument_WillSave::Params&& params) {
            std::filesystem::path str = params.textDocument.uri.path();
            config[str] = str;

            code[str] = urcl::source(documents[str], config[str]);
            code[str].updateReferences(code, config[str]);
            code[str].updateDefinitions(str);
            code[str].updateErrors(config[str]);
            lsp::notifications::TextDocument_PublishDiagnostics::Params errorParam{params.textDocument.uri, code[str].getDiagnostics()};
            messageHandler.sendNotification<lsp::notifications::TextDocument_PublishDiagnostics>(std::move(errorParam));
        }
    ).add<lsp::notifications::TextDocument_DidChange>(
        [&code, &config, &documents](lsp::notifications::TextDocument_DidChange::Params&& params) {
            for (lsp::TextDocumentContentChangeEvent change : params.contentChanges) {
                lsp::TextDocumentContentChangeEvent_Text fullChange = std::get<lsp::TextDocumentContentChangeEvent_Text>(change);
                std::vector<std::string> document = splitString(fullChange.text);
                std::filesystem::path str = params.textDocument.uri.path();
                code[str] = urcl::source(document, config[str]);
                code[str].updateReferences(code, config[str]);
                code[str].updateDefinitions(str);
                code[str].updateErrors(config[str]);
                documents[str] = std::move(document);
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
