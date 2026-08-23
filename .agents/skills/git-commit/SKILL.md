---
name: git-commit
description: 'Execute git commit with conventional commit message analysis, intelligent staging, and message generation. Use when user asks to commit changes, create a git commit, or mentions "/commit".'
license: MIT
allowed-tools: Bash
---

# Git Commit with Conventional Commits

## Overview

Create standardized, semantic git commits using the Conventional Commits specification. Analyze the actual diff to determine appropriate type, scope, and message.

## Conventional Commit Format

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

## Commit Types

| Type       | Purpose                        |
| ---------- | ------------------------------ |
| `feat`     | New feature                    |
| `fix`      | Bug fix                        |
| `docs`     | Documentation only             |
| `style`    | Formatting/style (no logic)    |
| `refactor` | Code refactor (no feature/fix) |
| `perf`     | Performance improvement        |
| `test`     | Add/update tests               |
| `build`    | Build system/dependencies      |
| `ci`       | CI/config changes              |
| `chore`    | Maintenance/misc               |
| `revert`   | Revert commit                  |

## Workflow

### 1. Analyze Diff

```bash
git diff --staged
git diff
git status --porcelain
```

### 2. Stage Files (if needed)

```bash
git add path/to/file1 path/to/file2
```

**Never commit secrets** (.env, credentials.json, private keys).

### 3. Generate Commit Message

- **Type**: What kind of change is this?
- **Scope**: What area/module is affected?
- **Description**: One-line summary of what changed (present tense, imperative mood, <72 chars)

### 3.5. Run Security Audit

Before executing commit, verify that no secrets, unintended files, or security risks are present by invoking the `security-auditor` skill or checking workspace files.

### 4. Execute Commit & Push

```bash
git commit -m "<type>[scope]: <description>"
git push
```

## Git Safety Protocol

- **NEVER commit or push without explicit user approval.** (Note: If this chat session closes, context is lost, so explicit user permission must be obtained afresh).
- NEVER update git config
- NEVER run destructive commands without explicit request
- NEVER skip hooks unless user asks
- NEVER force push to main/master
