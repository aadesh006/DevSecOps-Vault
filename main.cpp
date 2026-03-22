#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

const std::string RED = "\033[0;31m";
const std::string GREEN = "\033[0;32m";
const std::string YELLOW = "\033[1;33m";
const std::string NC = "\033[0m"; 

// AWS Access Key
const std::regex AWS_REGEX("AKIA[0-9A-Z]{16}");

// Stripe Live Token
const std::regex STRIPE_REGEX("sk_live_[0-9a-zA-Z]{24}");

// RSA Private Key: Matches the standard header of a private SSH/Encryption key.
const std::regex RSA_REGEX("-----BEGIN (RSA|OPENSSH) PRIVATE KEY-----");

const std::vector<std::pair<std::string, std::regex>> SECRET_PATTERNS = {
    {"AWS Access Key", AWS_REGEX},
    {"Stripe Live Token", STRIPE_REGEX},
    {"Private SSH/RSA Key", RSA_REGEX}
};

bool scanFile(const std::string& filepath) {
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << RED << "[ERROR]" << NC << " Could not open: " << filepath << "\n";
        return false; 
    }

    std::string line;
    int lineNumber = 1;
    bool leakFound = false;

    // Stream the file line by line
    while (std::getline(file, line)) {
        
        // Check the current line against every regex pattern in our dictionary
        for (const auto& patternPair : SECRET_PATTERNS) {
            const std::string& secretName = patternPair.first;
            const std::regex& regexPattern = patternPair.second;

            // std::regex_search returns true if the pattern exists ANYWHERE in the line
            if (std::regex_search(line, regexPattern)) {
                std::cout << RED << secretName << " DETECTED! " << NC 
                          << "\n   -> File: " << filepath 
                          << "\n   -> Line: " << lineNumber << "\n\n";
                leakFound = true;
            }
        }
        lineNumber++;
    }

    return leakFound;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 0; 
    }

    bool commitBlocked = false;

    // Loop through all staged files
    for (int i = 1; i < argc; ++i) {
        std::string filepath = argv[i];
        if (scanFile(filepath)) {
            commitBlocked = true;
        }
    }

    if (commitBlocked) {
        return 1;
    }

    return 0;
}