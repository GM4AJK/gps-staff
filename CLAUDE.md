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

## ESP32 builds and flashing

- ESP32 firmware (under `firmware/esp32-*/`) is built and flashed via `idf.py` directly in WSL — Claude owns this.
- Use `scripts/idf.sh` as a drop-in for `idf.py` — it sets the environment internally, no sourcing needed:
  ```
  cd firmware/esp32-handheld && /mnt/c/Users/kirkh/github/gps-staff/scripts/idf.sh build
  ```
- Handheld device node: `/dev/esp32_handheld` — flash with `idf.py -p /dev/esp32_handheld flash`.
- Zero (base/rover) device nodes: `/dev/esp32_base`, `/dev/esp32_rover`.

## Bench debugging / serial console

- The Nucleo boards log debug output (`app_log()`) over UART, normally viewed by the user via PuTTY on Windows.
- In a WSL session, there is no direct access to Windows COM ports (no USB/IP passthrough configured) — bench log lines must be pasted into the conversation by the user.
- In a native Windows PowerShell session, the COM port can be opened directly (e.g. .NET `System.IO.Ports.SerialPort` or pyserial) to capture a timed snapshot of debug output — but only one process can hold the port open, so ask the user to close their PuTTY session first.
- The user does not build firmware via the CLI — builds/flashes are done in STM32CubeIDE. Don't run `make`/build commands; rely on the user to flash and report results (or, from PowerShell, read them directly from the serial port as above).
