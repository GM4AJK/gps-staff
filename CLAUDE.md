# Working with this repo

## GitHub access

- SSH access to GitHub does not work from this environment (`Permission denied (publickey)`).
- All GitHub interaction must go through the **`gh` CLI**, authenticated with a fine-grained PAT (`gh auth status` confirms it's active). Do not attempt `git push`/`git pull`/`git fetch` over SSH or plain HTTPS — they will fail or bypass the PAT.
- Local `git` commands that don't touch the remote (status, diff, log, add, commit, checkout, branch) are fine to use directly.
- Operations that need the remote (creating branches, pushing commits, opening issues/PRs, merging) must be done via `gh` — e.g. `gh api` for git data/contents operations, `gh issue`, `gh pr`.

## Branch protection -- PRs only

- **A ruleset on GitHub requires all changes to land via pull request — direct commits/pushes to `main` are blocked.**
- Never commit or push straight to `main`. For any change:
  1. Create a feature branch (e.g. via `gh api repos/<owner>/<repo>/git/refs`).
  2. Commit the change to that branch (e.g. via `gh api repos/<owner>/<repo>/contents/<path>` or by pushing a local branch through `gh`).
  3. Open a PR with `gh pr create`.
  4. Merge with `gh pr merge` once ready (or leave for the user to review/merge).
- This applies to every change, however small — no exceptions for "quick fixes" or test commits.
