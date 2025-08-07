#pragma once
#include "Common.h"
#include <map>

class ArgParser {
public:
    ArgParser(int argc, char* argv[]);

    bool optionExists(const std::string& option) const;
    const std::string& getOption(const std::string& option) const;

private:
    std::vector<std::string> tokens;
    std::map<std::string, std::string> options;
};