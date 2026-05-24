# GitHub Actions Runner Configuration

This document describes how to install, configure and run the self-hosted GitHub Actions Runner on the Debian 12 server in order to automate Prysma performance tests.

## 1. Prerequisites on the GitHub Platform

1. Go to your code repository on **GitHub**.
2. Click on the **Settings** tab at the top.
3. In the left sidebar, click on **Actions** then on **Runners**.
4. Click the green **New self-hosted runner** button.
5. Select the platform: **Linux** and the architecture **x64**.
6. Keep this page open, it contains the download links and the unique registration token.

## 2. Installation on the OptiPlex Server

Connect via SSH to your server and run the following commands in your user directory:

```bash
# 1. Create a dedicated folder for the runner and navigate to it
mkdir ~/actions-runner && cd ~/actions-runner

# 2. Download the official runner archive (replace XXXX with the current version indicated by GitHub)
curl -o actions-runner-linux-x64-XXXX.tar.gz -L https://github.com/actions/runner/releases/download/vXXXX/actions-runner-linux-x64-XXXX.tar.gz

# 3. Extract the archive
tar xzf ./actions-runner-linux-x64-XXXX.tar.gz
```

## 3. Configuration and Registration

Run the configuration script, replacing the URL and token with those displayed on the GitHub page from Step 1:

```bash
./config.sh --url https://github.com/YOUR_USERNAME/YOUR_REPOSITORY --token YOUR_UNIQUE_TOKEN
```

*During the configuration, you can accept all proposed default options by pressing the **Enter** key.*

## 4. Interactive Execution (Temporary Test)

To quickly test that the runner connects properly to GitHub, you can launch it interactively in your terminal:

```bash
./run.sh
```

> [!WARNING]
> This mode is purely temporary. If you close your terminal or press **Ctrl+C** (which will display the message `Exiting...`), the runner will stop immediately and disconnect from GitHub. For production use, configure the permanent service below.

## 5. Permanent Execution (System Service)

To ensure that the runner runs automatically in the background (without blocking your terminal) and survives server restarts:

```bash
# Install the runner as a systemd service
sudo ./svc.sh install

# Start the service in the background
sudo ./svc.sh start

# Check the service status (the status should be "active (running)")
sudo ./svc.sh status
```

Once the service is started, the runner will appear in green with the `Idle` state on the GitHub platform (**Settings** -> **Actions** -> **Runners** tab), ready to receive and execute your continuous integration jobs.

> [!IMPORTANT]
> **Security for public repository:**
> If your GitHub repository is public, running the runner under your main user account presents a critical security risk (external contributors could execute arbitrary commands). It is strongly recommended to follow the steps in the [Runner Security](securisation_runner.md) guide to isolate it under a dedicated user and restrict allowed commands.
