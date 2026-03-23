# DevSecOps Vault

> A blazing-fast, C++-powered Git pre-commit hook that intercepts hardcoded secrets *before* they ever reach version control.

---

## The Problem

Credential leakage is one of the leading causes of enterprise security breaches. Most secret scanning tools operate *after* a push to a remote — by which point the damage is already done. **DevSecOps Vault** moves that checkpoint to the very first line of defense: your local machine, at the moment of `git commit`.

---

## How It Works

When you run `git commit`, the Vault silently activates and runs through a four-stage pipeline:

```
git commit
    │
    ▼
[1] pre-commit hook (Bash)
    │  Captures staged files via `git diff --cached`
    │
    ▼
[2] vault_scanner (C++ Binary)
    │  Streams each file line-by-line
    │  Evaluates against the secret pattern dictionary
    │
    ▼
[3] Mutator (C++ File I/O)
    │  Extracts the raw secret → writes to .env
    │  Rewrites source file with the correct env variable syntax
    │
    ▼
[4] Gatekeeper
       Returns exit code 0 (clean) or 1 (blocked) back to Git
```

---

## Key Features

- **Sub-10ms Execution** — The core scanning engine is compiled C++, so it never interrupts your workflow.
- **Multi-Pattern Detection** — Regex-based detection for AWS Access Keys (`AKIA...`), Stripe Live Tokens (`sk_live_...`), with an extensible pattern dictionary.
- **Language-Aware Mutation** — The mutator reads the file extension and injects the correct syntax automatically:

  | Extension      | Replacement Syntax                          |
  |----------------|---------------------------------------------|
  | `.js` / `.ts`  | `process.env.VAULT_SEC_<timestamp>`         |
  | `.py`          | `os.environ.get('VAULT_SEC_<timestamp>')`   |
  | `.cpp`         | `std::getenv("VAULT_SEC_<timestamp>")`      |
  | Other          | `<VAULT_SEC_<timestamp>_INSERT_ENV_HERE>`   |

- **Automatic .env Vaulting** — Detected secrets are timestamped, named uniquely, and appended to a local `.env` file that stays untracked.
- **Binary Stream Overwriting** — Uses binary file streams to overwrite source files, avoiding OS file-lock issues.
- **One-Command Setup** — The installer handles compilation and Git hook wiring automatically.

---

## Installation

### Prerequisites

- A Unix-like environment (Linux / macOS)
- `g++` with C++11 support or later
- An initialized Git repository

### Steps

**1. Clone or copy the source into your project root.**

**2. Compile the C++ engine:**
```bash
g++ main.cpp -o vault_scanner
```

**3. Run the installer:**
```bash
./install.sh
```

The installer will:
- Verify a `.git` directory is present
- Copy the pre-commit hook to `.git/hooks/pre-commit`
- Set executable permissions on the hook

That's it. The Vault is now active on every commit.

---

## Usage

Work normally. The Vault runs invisibly in the background.

```bash
git add script.py
git commit -m "Added database connection"
```

**If a clean commit — no output, proceeds normally:**
```
[Vault Scanner] Intercepting commit...
[INFO] Handing files over to the C++ Engine...
[OK] Code is clean. Committing...
```

**If a secret is detected, the Vault freezes the commit:**
```
[Vault Scanner] Intercepting commit...
[INFO] Handing files over to the C++ Engine...

AWS Access Key DETECTED!
   -> File: script.py (Line 14)
   -> Move this secret to the .env Vault? (y/n): y
   [OK] Line 14 mutated. Secret replaced with: os.environ.get('VAULT_SEC_1774205364')

[FATAL ERROR] Commit blocked. Please review vaulted files and re-stage them.
```

After vaulting, simply re-stage the mutated files and commit again:
```bash
git add .
git commit -m "Added database connection"
```

---

## Project Structure

```
devsecops-vault/
├── main.cpp              # Core C++ scanning and mutation engine
├── install.sh            # Automated installer and hook wiring script
├── hooks_source/
│   └── pre-commit        # Git pre-commit hook (Bash)
├── test.js               # Sample file for testing detection
└── README.md
```

---

## Extending the Pattern Dictionary

To add new secret types, open `main.cpp` and append to `SECRET_PATTERNS`:

```cpp
const std::regex GITHUB_REGEX("ghp_[0-9a-zA-Z]{36}");

const std::vector<std::pair<std::string, std::regex>> SECRET_PATTERNS = {
    {"AWS Access Key",    AWS_REGEX},
    {"Stripe Live Token", STRIPE_REGEX},
    {"GitHub PAT",        GITHUB_REGEX},  // ← add new patterns here
};
```

Recompile after any changes:
```bash
g++ main.cpp -o vault_scanner
```

---

## Tech Stack

| Layer              | Technology                                      |
|--------------------|-------------------------------------------------|
| Core Engine        | C++11 (STL, `<regex>`, `<fstream>`, `<ctime>`) |
| Hook Orchestration | Bash, Git Internals (`git diff --cached`)       |
| Terminal UX        | ANSI Escape Codes                               |
| Process Control    | OS-level exit codes (`0` / `1`)                 |

---

Built by [Aadesh Chaudhari](https://github.com/aadesh006)