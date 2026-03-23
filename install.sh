#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}========================================${NC}"
echo -e "${BLUE}    Installing DevSecOps Vault...${NC}"
echo -e "${CYAN}========================================${NC}\n"

# Verify Git repository
echo -e "${YELLOW}[INFO]${NC} Verifying Git repository environment..."
if [ ! -d ".git" ]; then
  echo -e "${RED}[FATAL]${NC} No .git directory found! Run this script from the root of a Git repository."
  exit 1
fi
echo -e "${GREEN}[OK]${NC} Git repository detected.\n"

# Verify g++ is available
echo -e "${YELLOW}[INFO]${NC} Checking for g++ compiler..."
if ! command -v g++ &> /dev/null; then
  echo -e "${RED}[FATAL]${NC} g++ not found. Please install a C++ compiler (e.g. 'sudo apt install g++' or 'xcode-select --install') and re-run."
  exit 1
fi
echo -e "${GREEN}[OK]${NC} g++ found: $(g++ --version | head -n1)\n"

# Compile the C++ engine
echo -e "${YELLOW}[INFO]${NC} Compiling vault_scanner..."
g++ -std=c++11 -O2 main.cpp -o vault_scanner

if [ $? -ne 0 ]; then
  echo -e "${RED}[FATAL]${NC} Compilation failed. Fix the error above and re-run install.sh."
  exit 1
fi
echo -e "${GREEN}[OK]${NC} vault_scanner compiled successfully.\n"

# Install the pre-commit hook
echo -e "${YELLOW}[INFO]${NC} Wiring up the pre-commit hook..."
cp hooks_source/pre-commit .git/hooks/pre-commit

if [ $? -ne 0 ]; then
  echo -e "${RED}[FATAL]${NC} Failed to copy the hook script to .git/hooks/."
  exit 1
fi
echo -e "${GREEN}[OK]${NC} Hook copied successfully.\n"

# Set executable permissions
echo -e "${YELLOW}[INFO]${NC} Setting executable permissions..."
chmod +x .git/hooks/pre-commit
echo -e "${GREEN}[OK]${NC} Permissions set.\n"

# Ensure .env is gitignored
echo -e "${YELLOW}[INFO]${NC} Checking .gitignore for .env and vault_scanner..."
GITIGNORE_UPDATED=false

if ! grep -qxF '.env' .gitignore 2>/dev/null; then
  echo '.env' >> .gitignore
  echo -e "${GREEN}[OK]${NC} Added .env to .gitignore."
  GITIGNORE_UPDATED=true
fi

if ! grep -qxF 'vault_scanner' .gitignore 2>/dev/null; then
  echo 'vault_scanner' >> .gitignore
  echo -e "${GREEN}[OK]${NC} Added vault_scanner to .gitignore."
  GITIGNORE_UPDATED=true
fi

if [ "$GITIGNORE_UPDATED" = false ]; then
  echo -e "${GREEN}[OK]${NC} .gitignore already up to date."
fi

echo -e "\n${GREEN} Installation complete! The Vault is now actively guarding your commits.${NC}\n"