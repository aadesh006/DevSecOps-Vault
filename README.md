# DevSecOps Vault

A blazing-fast, C++ powered Git pre-commit hook that proactively intercepts, vaults, and mutates hardcoded secrets before they reach version control.

## Overview
Credential leakage is one of the leading causes of enterprise security breaches. **DevSecOps Vault** acts as a local emergency brake for developers. Instead of relying on cloud-based secret scanning *after* a push, this tool hooks directly into the local Git OS process. 

If a developer accidentally stages a high-entropy secret (like an AWS Access Key or Stripe Token), the Vault freezes the commit in milliseconds, extracts the secret to a local, untracked `.env` file, and automatically rewrites the original source code to use safe environment variables.

## Key Features
* **Ultra-Low Latency Engine:** The core scanning engine is compiled in C++, ensuring the pre-commit hook executes in under 10ms so it doesn't interrupt the developer workflow.
* **Advanced Regex Detection:** Utilizes the C++ `<regex>` library to hunt down high-entropy strings and mathematical patterns matching AWS, Stripe, GitHub, Slack, and Google Cloud credentials.
* **Language-Aware Mutator:** The auto-mutator intelligently reads file extensions (`.js`, `.ts`, `.py`, `.cpp`) and injects the correct syntax for that specific language (e.g., `process.env.VAR`, `os.environ.get()`, `std::getenv()`).
* **Frictionless Vaulting:** Automatically generates a secure `.env` file, creates a unique time-stamped variable, and safely scrubs the original source file via binary stream overwriting to prevent OS file-lock bugs.
* **Seamless Distribution:** Includes an automated bash installer that wires up the OS-level Git hooks without requiring manual developer configuration.

## System Architecture

The tool orchestrates communication between Git, the Operating System, and the compiled C++ binary:

1. **The Interceptor (`pre-commit` shell script):** Git triggers the shell script upon `git commit`. The script queries `git diff --cached` and passes staged files to the engine.
2. **The Scanner (`vault_scanner` C++ binary):** Streams file contents line-by-line (for memory efficiency) and evaluates them against the threat dictionary.
3. **The Mutator (C++ File I/O):** If a threat is found, standard input (`/dev/tty`) is reattached for interactive prompting. Upon approval, it utilizes binary file streams to overwrite the source code and update the `.env` vault.
4. **The Gatekeeper:** Returns an OS-level exit code (`0` for success, `1` for blocked) back to Git to finalize or abort the commit process.

## Installation

1. Clone this repository or download the source code into your project root.
2. Compile the C++ scanning engine:
   ```bash
   g++ main.cpp -o vault_scanner
   ```
3. Run the automated installer to wire up the Git hooks:
   ```bash
   ./install.sh
   ```

## Usage

Once installed, the Vault runs invisibly in the background. Just commit your code normally:

```bash
git add script.py
git commit -m "Added new database connection"
```
If a secret is detected, the Vault will intercept the terminal:
```plaintext
[Vault Scanner] Intercepting commit...
[INFO] Handing files over to the C++ Engine...

AWS Access Key DETECTED! 
   -> File: script.py (Line 14)
   -> Move this secret to the .env Vault? (y/n): y
   [OK] Line 14 mutated. Secret replaced with: os.environ.get('VAULT_SEC_1774205364')

[FATAL ERROR] Commit blocked. Please review vaulted files and re-stage them.
Simply run git add . to stage the newly mutated, safe files, and run your commit again!
```
## Tech Stack

* **Core Engine: C++ (Standard Template Library, <regex>, <fstream>)**
* **Process Management: Bash Shell Scripting, Git Internals, ANSI Escape Codes**
* **Architecture: Custom CLI Tooling, OS-level Exit Code routing**

Built by Aadesh Chaudhari
