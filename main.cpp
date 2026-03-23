#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <cstdlib>
#include <ctime>

const std::string RED    = "\033[0;31m";
const std::string GREEN  = "\033[0;32m";
const std::string YELLOW = "\033[1;33m";
const std::string CYAN   = "\033[0;36m";
const std::string NC     = "\033[0m";

const std::vector<std::pair<std::string, std::regex>> SECRET_PATTERNS = {
 
    { "AWS Access Key ID",
      std::regex("AKIA[0-9A-Z]{16}") },
 
    { "Google API Key",
      std::regex("AIza[0-9A-Za-z\\-_]{35}") },
 
    { "Stripe Live Secret Key",
      std::regex("sk_live_[0-9a-zA-Z]{24,}") },
 
    { "Stripe Test Secret Key",
      std::regex("sk_test_[0-9a-zA-Z]{24,}") },
 
    { "GitHub Personal Access Token (classic)",
      std::regex("ghp_[A-Za-z0-9]{36,}") },
 
    { "GitHub Fine-Grained PAT",
      std::regex("github_pat_[A-Za-z0-9_]{82,}") },
 
    { "Slack Bot Token",
      std::regex("xoxb-[0-9]{10,13}-[0-9]{10,13}-[A-Za-z0-9]{24,26}") },
 
    { "Slack User Token",
      std::regex("xoxp-[0-9A-Za-z\\-]{60,}") },
 
    { "Slack Incoming Webhook",
      std::regex("https://hooks\\.slack\\.com/services/T[A-Za-z0-9_]+/B[A-Za-z0-9_]+/[A-Za-z0-9_]+") },
 
    { "OpenAI API Key",
      std::regex("sk-[A-Za-z0-9]{48,}") },
 
    { "Anthropic API Key",
      std::regex("sk-ant-[A-Za-z0-9\\-_]{90,}") },
 
    { "SendGrid API Key",
      std::regex("SG\\.[A-Za-z0-9_\\-]{22,}\\.[A-Za-z0-9_\\-]{40,}") },
 
    { "Twilio Account SID",
      std::regex("AC[a-f0-9]{32}") },
 
    { "Mailgun API Key",
      std::regex("key-[a-zA-Z0-9]{32}") },
 
    { "NPM Access Token",
      std::regex("npm_[A-Za-z0-9]{36,}") },

    { "PEM Private Key Block",
      std::regex("-----BEGIN [A-Z ]*PRIVATE KEY-----") },
 
};

// Global counter to guarantee unique variable names even within the
static int vaultCounter = 0;

std::string generateVaultVarName() {
    return "VAULT_SEC_" + std::to_string(std::time(0)) + "_" + std::to_string(++vaultCounter);
}

std::string buildSafeReplacement(const std::string& filepath,
                                  const std::string& vaultVarName) {
    if (filepath.find(".js")  != std::string::npos ||
        filepath.find(".ts")  != std::string::npos) {
        return "process.env." + vaultVarName;
    } else if (filepath.find(".py") != std::string::npos) {
        return "os.environ.get('" + vaultVarName + "')";
    } else if (filepath.find(".cpp") != std::string::npos) {
        return "std::getenv(\"" + vaultVarName + "\")";
    } else {
        return "<" + vaultVarName + "_INSERT_ENV_HERE>";
    }
}

std::string stripQuotedSecret(const std::string& line,
                               const std::string& rawSecret,
                               const std::string& safeReplacement) {
    std::string result = line;

    // Try to find the secret wrapped in double quotes first, then single quotes.
    for (char q : {'"', '\''}) {
        std::string quoted = std::string(1, q) + rawSecret + std::string(1, q);
        size_t pos = result.find(quoted);
        if (pos != std::string::npos) {
            result.replace(pos, quoted.size(), safeReplacement);
            return result;
        }
    }

    // Fallback: secret is not quoted — replace it bare.
    size_t pos = result.find(rawSecret);
    if (pos != std::string::npos) {
        result.replace(pos, rawSecret.size(), safeReplacement);
    }
    return result;
}

bool scanAndMutateFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::string tempFilepath = filepath + ".tmp";
    std::ofstream tempFile(tempFilepath);

    std::string line;
    int lineNumber = 1;
    bool leakFoundAndBlocked = false;

    while (std::getline(file, line)) {
        for (const auto& patternPair : SECRET_PATTERNS) {
            std::smatch match;

            if (std::regex_search(line, match, patternPair.second)) {
                std::cout << RED << "\n" << patternPair.first << " DETECTED! " << NC
                          << "\n   -> File: " << filepath << " (Line " << lineNumber << ")\n";

                std::cout << CYAN << "   -> Move this secret to the .env Vault? (y/n): " << NC;
                char choice;
                std::cin >> choice;

                if (choice == 'y' || choice == 'Y') {
                    std::string rawSecret    = match.str(0);
                    std::string vaultVarName = generateVaultVarName(); // FIX 1
                    std::string safeReplacement = buildSafeReplacement(filepath, vaultVarName);

                    // Append the raw secret to .env
                    std::ofstream envFile(".env", std::ios_base::app);
                    envFile << vaultVarName << "=\"" << rawSecret << "\"\n";
                    envFile.close();

                    line = stripQuotedSecret(line, rawSecret, safeReplacement);

                    std::cout << GREEN << "  [OK] Line " << lineNumber
                              << " mutated. Secret replaced with: "
                              << CYAN << safeReplacement << NC << "\n";

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
        // Overwrite the original file using binary streams
        std::ifstream src(tempFilepath, std::ios::binary);
        std::ofstream dst(filepath,     std::ios::binary | std::ios::trunc);
        dst << src.rdbuf();
        src.close();
        dst.close();
    }

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
        std::cout << RED << "\n[FATAL ERROR] Commit blocked. Please review vaulted files and re-stage them.\n"
                  << "Simply run: git add . — then commit again.\n" << NC;
        return 1;
    }

    return 0;
}