## Overview
This file describes how the Agent works when fixing bugs and building new features. The Agent runs in **autonomous mode**, makes best judgments, and delivers work without waiting for extra input.

---

## Rules for Development
- Work in **full autonomous mode**: do not expect input from users.
- Always **make progress**: use timeouts and fallback steps if blocked.
- Use the **best available tools** (linters, formatters, test runners, analyzers).
- Each issue or feature must be handled in its **own branch and Pull Request**.
- Do not mix multiple fixes/features into one PR.

---

## Commits and Pull Requests
- you can choose any format for commits and creating pull requests.
- create a separate commit and push for an every fix.

