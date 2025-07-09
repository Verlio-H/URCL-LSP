#include <lsp/messages.h>
#include <lsp/connection.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>

#include "urcl/source.h"
#include "urcl/config.h"

#include <lsp/types.h>

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

int main() {
    lsp::Connection connection = lsp::Connection(lsp::io::standardIO());
    lsp::MessageHandler messageHandler = lsp::MessageHandler(connection);
    std::unordered_map<std::filesystem::path, urcl::source> code;
    std::unordered_map<std::filesystem::path, urcl::config> config;
    std::unordered_map<std::filesystem::path, std::vector<std::string>> documents;

    messageHandler.add<lsp::requests::Initialize>(
        [](lsp::requests::Initialize::Params&& params) {
            return lsp::requests::Initialize::Result{
                .capabilities = {
                    .positionEncoding = lsp::PositionEncodingKind::UTF16,
                    .textDocumentSync = lsp::TextDocumentSyncOptions(true, lsp::TextDocumentSyncKind::Full, true),
                    .definitionProvider = true,
                    .foldingRangeProvider = true,
                    .semanticTokensProvider = lsp::SemanticTokensOptions(false, {{"keyword", "variable", "number", "function", "comment", "class", "operator", "macro", "string", "escape", "operator", "namespace"}, {}}, false, true)
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
            config.emplace(str, str);

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
    );
    
    while (true) {
        messageHandler.processIncomingMessages();
    }

    return 0;
}