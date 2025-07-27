#include "config.h"
#include "../util.h"

#include <fstream>

urcl::config::config() {}

urcl::config::config(std::filesystem::path file, const std::filesystem::path& src) {
    std::ifstream in(file);
    std::string line;

    useCore = true;
    useBasic = true;
    useComplex = true;
    bool foundIncludes = false;

    while (std::getline(in, line)) {

        line = util::trim(line);

        bool set = true;
        switch (line[0]) {
            case ('#'): { // comment
                continue;
            }
            case ('-'): // remove config option
                set = false;
            case ('+'): { // add config option
                std::string_view value = std::string_view(line).substr(1);
                if (value == "core") {
                    useCore = set;
                } else if (value == "basic") {
                    useBasic = set;
                } else if (value == "complex") {
                    useComplex = set;
                } else if (value == "standard") {
                    useStandard = set;
                } else if (value == "iris") {
                    useIris = set;
                } else if (value == "urcx") {
                    useUrcx = set;
                } else if (value == "irix") {
                    useUrcx = set;
                    useIris = set;
                } else if (value == "lowercase") {
                    useLowercase = set;
                } else if (value == "uir") {
                    useUir = set;
                }
                break;
            }
            default: // path
                if (foundIncludes) continue;
                std::vector<std::filesystem::path> paths;
                while (line != "") {
                    int end;
                    for (end = 0; end < line.length(); ++end) {
                        if (line[end] == '\\' && end < line.length() - 1 && line[end + 1] == ' ') {
                            ++end;
                        } else if (line[end] == ' ') {
                            break;
                        }
                    }
                    std::string pathEnd;
                    pathEnd = line.substr(0, end);
                    util::replaceAll(pathEnd, "\\ ", " ");

                    if (end < line.length() - 1) {
                        line = util::trim(line.substr(end + 1));
                    } else {
                        line = "";
                    }

                    paths.push_back(file.parent_path()/pathEnd);
                }
                for (int i = 0; i < paths.size(); ++i) {
                    std::filesystem::path& path = paths[i];
                    if (std::filesystem::exists(path) && std::filesystem::equivalent(path, src)) {
                        paths.erase(paths.begin() + i);
                        foundIncludes = true;
                        this->includes = std::move(paths);
                        break;
                    }
                }
        }

    }
}

urcl::config::config(const std::filesystem::path file) {
    useCore = true;
    useBasic = true;
    useComplex = true;
    useIris = true;
    useUrcx = true;
    useUir = true;
    std::filesystem::path path = file.parent_path();
    while (path != file.root_path()) {
        if (std::filesystem::exists(path/"lsp.txt")) {
            urcl::config trueConfig(path/"lsp.txt", file);
            this->useCore = trueConfig.useCore;
            this->useBasic = trueConfig.useBasic;
            this->useComplex = trueConfig.useComplex;
            this->useStandard = trueConfig.useStandard;
            this->useIris = trueConfig.useIris;
            this->useUrcx = trueConfig.useUrcx;
            this->useUir = trueConfig.useUir;
            this->useLowercase = trueConfig.useLowercase;
            this->includes = std::move(trueConfig.includes);
            if (file.extension() == ".uir") useUir = true;
            return;
        }
        path = path.parent_path();
    }
}
