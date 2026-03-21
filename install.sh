#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color (Reset)

echo -e "${CYAN}========================================${NC}"
echo -e "${BLUE}    Installing DevSecOps Vault...${NC}"
echo -e "${CYAN}========================================${NC}\n"

#Check for Git Repository
echo -e "${YELLOW}[INFO]${NC} Verifying Git repository environment..."
if [ ! -d ".git" ]; then
  echo -e "${RED}[FATAL]${NC} No .git directory found! You must run this script from the root of a Git repository."
  exit 1
fi
echo -e "${GREEN}[OK]${NC} Git repository detected.\n"

#Install the Hook
echo -e "${YELLOW}[INFO]${NC} Wiring up the pre-commit hook..."
cp hooks_source/pre-commit .git/hooks/pre-commit

# Safety Check: Did the copy command actually work?
# $? captures the exit code of the last run command (cp in this case)
if [ $? -ne 0 ]; then
  echo -e "${RED}[FATAL]${NC} Failed to copy the hook script to .git/hooks/."
  exit 1
fi
echo -e "${GREEN}[OK]${NC} Hook copied successfully.\n"

#Make it Executable
echo -e "${YELLOW}[INFO]${NC} Setting executable permissions..."
chmod +x .git/hooks/pre-commit

echo -e "\n${GREEN} Installation complete! The Vault is now actively guarding your commits.${NC}"