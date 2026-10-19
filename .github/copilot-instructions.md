# Project Guidelines

## Target Platform

- Target a Raspberry Pi 2 Model B running DietPi with a 32-bit `armv7l` userspace.
- Build native code on the Raspberry Pi. Do not use the existing `aarch64` WSL cross-build tasks for this device.
- Before building, verify that `uname -m` reports `armv7l` and inspect `/etc/os-release` to confirm a DietPi or Debian-compatible system. Stop and report the mismatch if either assumption is false.

## Implementation

- Prefer C for new application code. Use the C API in `include/led-matrix-c.h` and follow `examples-api-use/c-example.c` for local style and lifecycle patterns.
- If a feature requires HTTP, web APIs, or other web-specific behavior, implement that boundary as a Go library with an explicit C-callable interface and call it from C. Document its build integration, dependencies, memory ownership, and error behavior. Do not introduce Go when no web behavior is required.
- Keep changes focused and update the nearest relevant README when setup, dependencies, build commands, runtime flags, or user-visible behavior changes.
- In C, write concise diagnostics to standard error with `fprintf(stderr, ...)`, following the existing examples. In a Go web library, use standard-library logging. Keep normal output separate from diagnostics and never log credentials, passwords, private-key paths, tokens, or secret endpoint data.

## Remote Connection

- Collect connection details only when a remote build is needed. Ask the user for the device hostname or IP address, SSH username, and optional private-key path. Do not hardcode, commit, or persist these values.
- Build the SSH destination as `user@host`. Quote an identity path and pass it with `ssh -i` when supplied.
- Never ask the user to send a password, private-key passphrase, token, or other secret through chat. When SSH or rsync requests an account password or private-key passphrase, explicitly tell the user to enter it directly into that terminal prompt. Do not provide, relay, echo, log, or persist the secret.
- Establish one session-scoped OpenSSH multiplexed connection with `ControlMaster=yes`, `ControlPersist=yes`, and a temporary control socket. Verify it with a cheap remote command, then use the same socket for all SSH and rsync operations in the work session.
- Keep the master connection open through repository synchronization, building, and verification. Close it explicitly with `ssh -O exit` when the remote work is complete. On authentication failure, report the error and stop instead of retrying repeatedly.

## Remote Repository

- Use `~/rpi-rgb-led-matrix` by default. If it is not a Git repository, ask whether to use an existing alternate path or clone the repository into that default location. Validate an alternate path before continuing.
- Determine the active local branch and repository origin. Before changing the remote checkout, inspect its branch and working-tree status. If the remote worktree has changes, stop and ask how to proceed; never overwrite or discard them.
- Fetch remote refs and check out the branch matching the active local branch when it exists. If it does not exist remotely, report that fact and ask before creating it or choosing another branch.
- Show the resolved destination, repository path, and branch before synchronization.
- Synchronize the local working tree with rsync so uncommitted local edits can be built. Preserve the remote `.git/` directory and exclude ignored or generated outputs such as object files, archives, shared libraries, `bin/`, `obj/`, and Python caches.
- Do not broadly delete remote files. If removal is required to mirror locally deleted tracked files, calculate the affected source paths, show them to the user, and obtain confirmation before deleting only those paths.

## Dependencies And Build

- Check for required commands before installing packages. For the default C target, install missing prerequisites on DietPi with `sudo apt-get update` followed by `sudo apt-get install -y build-essential git rsync`.
- Let `sudo` request authentication directly in the terminal. Do not install an ARM64 cross-compiler for this native Pi 2 workflow.
- Before adding optional packages, read the target's repository documentation. For example, image and video utilities have additional GraphicsMagick or FFmpeg development dependencies documented under `utils/`.
- If the user names an executable, locate its owning Makefile and build only that target. Otherwise, run `make -C examples-api-use c-example`; this also builds `lib/librgbmatrix.a` as needed.
- Prefer incremental builds. Run a clean build only when requested or when stale artifacts from another architecture or configuration are detected.
- Check the build exit status and inspect the resulting executable with `file`; it must be a 32-bit ARM artifact for this target.
- Do not run GPIO or matrix executables with elevated privileges unless the user explicitly requests execution. For initial Pi 2 runtime testing, suggest `--led-slowdown-gpio=1` rather than forcing a compile-time setting.