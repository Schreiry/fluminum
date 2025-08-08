#include "ArgParser.h"

ArgParser::ArgParser(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        this->tokens.push_back(std::string(argv[i]));
    }

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].rfind("--", 0) == 0) { // It's an option key
            const std::string& key = tokens[i];
            // Check if there's a next token and it's not another key
            if (i + 1 < tokens.size() && tokens[i + 1].rfind("--", 0) != 0) {
                options[key] = tokens[i + 1];
                i++; // Skip the value token
            }
            else {
                options[key] = ""; // It's a flag with no value
            }
        }
    }
}

bool ArgParser::optionExists(const std::string& option) const {
    return options.count(option);
}

const std::string& ArgParser::getOption(const std::string& option) const {
    auto it = options.find(option);
    if (it == options.end()) {
        throw std::runtime_error("Command line option not found: " + option);
    }
    return it->second;
}