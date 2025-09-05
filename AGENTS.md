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
- Create a **separate commit for every fix**.
- Push changes immediately after the fix is complete.
- Open a **PR for each issue** (in Gerrit or Git).
- Commit message format is flexible, but should be **clear and short**.
- PR description should explain:
  - What was broken or requested.
  - How it was fixed or implemented.
  - How it was tested.

---

## Workflow
1. Identify bug or feature from code/issues.
2. Make best judgment about the fix or design.
3. Implement the change with clean and simple code.
4. Add/update tests to prove correctness.
5. Push commit and open PR.

---

## Checklist Before PR
- [ ] Code formatted and linted.
- [ ] Tests added or updated.
- [ ] CI/tests passing.
- [ ] Separate commit and PR per fix/feature.
