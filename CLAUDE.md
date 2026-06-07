# Working with this repo

## GitHub access

- SSH access to GitHub does not work from this environment (`Permission denied (publickey)`).
- The `origin` remote is configured over **HTTPS** (`https://github.com/GM4AJK/gps-staff.git`). `gh auth setup-git` has been run, which makes git authenticate via `gh`'s credential helper using the fine-grained PAT (`gh auth status` confirms it's active). This means normal `git fetch`/`pull`/`push` work fine and are PAT-authenticated through `gh` — they are not bypassing it.
- For non-git-protocol GitHub operations (issues, pull requests, branch refs, file commits via the contents API, etc.), use the `gh` CLI / `gh api` directly.

## Branch protection -- PRs only

- **A ruleset on GitHub requires all changes to land via pull request — direct commits/pushes to `main` are blocked.** GitHub rejects a direct push to `main` regardless of how you authenticate (PAT, SSH, etc.) — this isn't a credential issue, it's enforced server-side.
- Never commit or push straight to `main`. For any change:
  1. Create a feature branch (`git checkout -b <name>`, or via `gh api repos/<owner>/<repo>/git/refs`).
  2. Commit the change to that branch (`git commit` + `git push -u origin <name>`, or via `gh api repos/<owner>/<repo>/contents/<path>`).
  3. Open a PR with `gh pr create`.
  4. Merge with `gh pr merge` once ready (or leave for the user to review/merge).
- This applies to every change, however small — no exceptions for "quick fixes" or test commits.

## Datasheets and reference documents

- Component datasheets and other reference documents (e.g. dev board schematics) are stored in `docs/datasheets/`.
- `docs/README.md` is a catalog of these documents — update its table whenever a document is added, moved, or removed.
