# Development environment integration tests

[Version française](README.FR.md)

The main entry point remains `tests/run_unit_tests.py`. Its original five steps are preserved.
One sixth step invokes the development environment orchestrator.

## Run

Python 3.12 is the CI reference. Install the test dependencies on the host and build the image from the repository root:

```sh
python -m pip install -r requirements.txt -r tests/environnement_dev_tests/requirements.txt
docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .
python tests/run_unit_tests.py --dev-env
```

Selections:

```sh
python tests/run_unit_tests.py                              # all six steps
python tests/run_unit_tests.py --existing                   # original five steps
python tests/run_unit_tests.py --dev-env --family common
python tests/run_unit_tests.py --dev-env --family linux
python tests/run_unit_tests.py --dev-env --family window
python tests/run_unit_tests.py --dev-env --family vscode
python tests/run_unit_tests.py --dev-env --family window --native-only
```

Repeat `--family` to select several families. A selected family fails if a required component is missing.
`--native-only` is an explicit partial run, excluding Docker and VS Code; it does not validate those components.
Tests belonging to another OS are not applicable. Independent selected families still run after a failure.

## Families and prerequisites

| Directory | Real components exercised |
| --- | --- |
| `common/integration/` | Docker image tools, volumes, UID/GID, read-only mounts, ccache misses/hits/rebuilds, errors, logs, debugpy DAP and LLDB remote debugging |
| `linux/integration/` | Bash launchers, occupied ports, duplicate launches, cancellation/relaunch, paths with spaces/accents, CMake compilation database and clangd LSP |
| `window/integration/` | Native PowerShell parser and missing-target handling; Docker Desktop mounts and exit codes when Docker tests are selected |
| `vscode/` | A real VS Code extension test host, Python and Prysma C++ breakpoints, pre/post tasks, and a failed build with an existing binary |
| `fixtures/` | Deterministic programs used by the debugger tests |

The shared protocol clients are small test helpers, without mocked Docker, debugpy or LLDB services.
The clangd scenario generates and builds a real CMake project in Docker, adapts its database, then verifies host clangd definition, hover, completion and diagnostic updates with an LLVM header.

Linux requires Docker Engine, Bash, Python, GNU timeout and host clangd. Windows Docker tests require native Python, PowerShell and Docker Desktop using Linux containers. Running PowerShell in Linux does not validate Windows.

VS Code tests require a graphical session (or `xvfb-run` on Linux), the `code` CLI, and network access to install pinned debugger extensions into a disposable profile. Set `PRYSMA_VSCODE_BIN` if needed. These tests copy the source tree, build Prysma in Docker with two jobs and intentionally break only that copy. They invoke the same debug API used by F5; keyboard input and visual UI rendering are not asserted.

## Isolation, deadlines and results

Test containers and volumes use unique names and a `prysma.test` label. Debug launchers derive their names from the workspace and reject conflicts. Only owned container IDs are removed. Ports bind to loopback. LLDB readiness checks the container's listening socket without consuming its debugger connection; the tests additionally validate protocol exchanges.

The Python launcher is now a background VS Code task, like LLDB. Start scripts remain alive until the debug container exits; `--stop` stops only that workspace's session. Both launchers remove their own container on exit or interruption.

The non-background `Prepare ... Debugging` tasks sequence compilation and server startup. This lets a failed compilation abort the launch before VS Code waits for the background server's readiness signal. `debug.onTaskErrors` is set to `abort` in the workspace.

Configuration:

| Setting | Default |
| --- | --- |
| `PRYSMA_TEST_IMAGE` | `prysma-debug` |
| `PRYSMA_TEST_COMMAND_TIMEOUT` | 90 seconds per test Docker command |
| `--family-timeout` | 1800 seconds per family |
| `PRYSMA_DEBUG_START_TIMEOUT` | 15 seconds for a server to listen |
| `PRYSMA_DOCKER_TIMEOUT` | 30 seconds per Bash launcher Docker operation |
| `PRYSMA_DEBUG_IMAGE` / `PRYSMA_DEBUG_PORT` | Default debug image and debugger port |
| `PRYSMA_BUILD_JOBS` | CPU count for normal builds; tests use 2 |

A developer paused at a breakpoint has no session timeout. The automated test family has its own deadline.
Results are stored under `build/dev-env-results/<run-id>/`: one log and JUnit XML per family, and `summary.json` with system information, component versions/image identity, selection, durations and exit codes.
Use `--results-dir PATH` to change the location. Pytest captures subprocess output for diagnostics.

## Windows CI

The WSL2 and nested virtualization portions of the Windows GitHub Actions workflow are **experimental**.
Native PowerShell tests use GitHub-hosted `windows-2022` runners. The WSL job uses `windows-2025`, which includes WSL2, and **Docker Engine inside an imported Ubuntu 24.04 WSL2 distribution**, not Docker Desktop.

Native PowerShell tests run in a separate job, even if WSL preparation fails. The WSL job checks the preinstalled WSL version and VirtualMachinePlatform feature, imports and boots WSL2, installs Docker Engine, and verifies a real container before building the development image. It does not reinstall or update WSL during the job: enabling Windows features can require a reboot, and the legacy WSL included in `windows-2022` cannot be assumed to support modern installation options. Preparation failures fail that job; they are not successful or skipped test runs. Tests only start after preparation succeeds.

The WSL job runs the `common`, `linux` and `vscode` families, including real Python and Prysma C++ breakpoints and rejection of a failed build with an existing binary. VS Code 1.96.4 runs under Xvfb as an ordinary Linux user with access to Docker. The checkout is copied to the distribution's Linux filesystem so that ownership and executable permissions have Linux semantics. The setup script is intended only for the fresh CI distribution, not a developer's machine.

WSL tests validate Linux behavior inside WSL, including the Linux VS Code application. This workflow does **not** validate Windows VS Code with Remote WSL, native Windows Docker Desktop debugging, a non-C: workspace or native Windows VS Code F5. Run the full `window` and `vscode` families on a Windows machine with Docker Desktop for native Windows coverage.

The workflow runs on pushes and pull requests. It can also be started from **Actions → CI Windows → Run workflow** once present on the default branch. Both jobs must succeed: `windows-native` checks native PowerShell, while `wsl-docker` checks the WSL environment. Download `windows-native-results` and `wsl-docker-results` from the run to inspect logs, JUnit results, `summary.json` and `vscode-scenarios.json`. WSL diagnostics are collected even after a test failure; failures before WSL boots remain visible in the preparation step's log. The dedicated distribution is stopped at the end.

The `wsl-docker-results` artifact includes `wsl-preflight.log` even when the WSL preflight check fails. Runner prerequisites are listed in the [Windows 2025 image documentation](https://github.com/actions/runner-images/blob/main/images/windows/Windows2025-Readme.md). WSL setup follows [Microsoft's command reference](https://learn.microsoft.com/en-us/windows/wsl/basic-commands). The VS Code test host uses the [documented extension test entry point](https://code.visualstudio.com/api/working-with-extensions/testing-extension).

## Remaining coverage limits

The supplied test plan is broader than this suite. Not yet covered: every breakpoint edge case (duplicate basenames, stale source, stripped/optimized binaries), every variable representation, partial LLVM header copies, project relocation, all VS Code task diagnostics/cancellation combinations, abrupt editor termination, and Flatpak UI end-to-end behavior. The Linux Flatpak Docker wrapper remains supported, but host tests are not evidence of a complete Flatpak UI validation.

The manual Linux and Windows guides supplement the automated tests; they are not recorded as automated passes.
