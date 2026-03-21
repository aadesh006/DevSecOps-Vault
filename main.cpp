#include <iostream>
#include <fstream>
#include <string>

const std::string RED = "\033[0;31m";
const std::string GREEN = "\033[0;32m";
const std::string YELLOW = "\033[1;33m";
const std::string NC = "\033[0m"; 

bool scanFile(const std::string& filepath) {
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << RED << "[ERROR]" << NC << " Could not open: " << filepath << "\n";
        return false; 
    }

    std::string line;
    int lineNumber = 1;

    // Stream the file line by line
    while (std::getline(file, line)) {
        if (line.find("AKIA") != std::string::npos || line.find("password") != std::string::npos) {
            std::cout << RED << "SECRET DETECTED! " << NC 
                      << "File: " << filepath << " | Line: " << lineNumber << "\n";
            return true;
        }
        lineNumber++;
    }

    return false;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 0; 
    }

    bool leakDetected = false;

    // Loop through all the files passed by the shell script
    for (int i = 1; i < argc; ++i) {
        std::string filepath = argv[i];
        
        if (scanFile(filepath)) {
            leakDetected = true;
        }
    }

    // This exit code
    if (leakDetected) {
        return 1; // Block the commit
    }

    return 0; // Allow the commit
}