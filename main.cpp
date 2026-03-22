#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <cstdlib> // For system commands
#include <ctime>   // For generating unique keys

const std::string RED = "\033[0;31m";
const std::string GREEN = "\033[0;32m";
const std::string YELLOW = "\033[1;33m";
const std::string CYAN = "\033[0;36m";
const std::string NC = "\033[0m"; 

const std::regex AWS_REGEX("AKIA[0-9A-Z]{16}");
const std::regex STRIPE_REGEX("sk_live_[0-9a-zA-Z]{24}");

const std::vector<std::pair<std::string, std::regex>> SECRET_PATTERNS = {
    {"AWS Access Key", AWS_REGEX},
    {"Stripe Live Token", STRIPE_REGEX}
};

bool scanAndMutateFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::string tempFilepath = filepath + ".tmp";
    std::ofstream tempFile(tempFilepath);
    
    std::string line;
    int lineNumber = 1;
    bool leakFoundAndBlocked = false;

    while (std::getline(file, line)) {
        bool lineMutated = false;

        for (const auto& patternPair : SECRET_PATTERNS) {
            std::smatch match;
            
            if (std::regex_search(line, match, patternPair.second)) {
                std::cout << RED << "\n" << patternPair.first << " DETECTED! " << NC 
                          << "\n   -> File: " << filepath << " (Line " << lineNumber << ")\n";
                
                std::cout << CYAN << "   -> Move this secret to the .env Vault? (y/n): " << NC;
                char choice;
                std::cin >> choice;

                if (choice == 'y' || choice == 'Y') {
                    std::string rawSecret = match.str(0);
                    
                    // Generate a unique variable name based on time
                    std::string vaultVarName = "VAULT_SEC_" + std::to_string(std::time(0));
                    
                    // Append the raw secret to the .env file
                    std::ofstream envFile(".env", std::ios_base::app);
                    envFile << vaultVarName << "=\"" << rawSecret << "\"\n";
                    envFile.close();

                    // Replace the raw secret in the code with the environment variable
                    std::string safeReplacement = "process.env." + vaultVarName;
                    line = std::regex_replace(line, patternPair.second, safeReplacement);
                    
                    std::cout << GREEN << "   [OK] Secret vaulted securely as " << vaultVarName << NC << "\n";
                    lineMutated = true;
                    leakFoundAndBlocked = true;
                } else {
                    std::cout << YELLOW << "   [WARNING] Secret left exposed. Blocking commit.\n" << NC;
                    leakFoundAndBlocked = true;
                }
            }
        }
        tempFile << line << "\n";
        lineNumber++;
    }

    file.close();
    tempFile.close();

    if (leakFoundAndBlocked) {
        std::ifstream src(tempFilepath, std::ios::binary);
        std::ofstream dst(filepath, std::ios::binary | std::ios::trunc);
        
        dst << src.rdbuf();
        
        src.close();
        dst.close();
    }
    
    //clean up the temporary file
    std::remove(tempFilepath.c_str()); 

    return leakFoundAndBlocked;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 0; 

    bool commitBlocked = false;

    for (int i = 1; i < argc; ++i) {
        if (scanAndMutateFile(argv[i])) {
            commitBlocked = true;
        }
    }

    if (commitBlocked) {
        std::cout << RED << "\n[FATAL ERROR] Commit blocked. Please review vaulted files and re-stage them.\n" << NC;
        return 1; 
    }

    return 0; 
}