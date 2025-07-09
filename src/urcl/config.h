#ifndef CONFIG_H
#define CONFIG_H

#include <filesystem>
#include <vector>

namespace urcl {
    class config {
        public:
            bool useCore;
            bool useBasic;
            bool useComplex;
            bool useIris;
            bool useUrcx;
            bool useStandard;
            std::vector<std::filesystem::path> includes;

            config(std::filesystem::path file);
            config();
        private:
            config(std::filesystem::path file, const std::filesystem::path& src);
    };
}

#endif