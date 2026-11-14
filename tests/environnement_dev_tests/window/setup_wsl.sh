#!/bin/bash
# CI-only provisioning, executed as root in the dedicated Prysma WSL distribution.
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y docker.io python3-venv clang-18 clangd-18 g++ curl ca-certificates \
    xvfb xauth libasound2t64 libgbm1 libgtk-3-0 libnss3 libxss1 libatk-bridge2.0-0 \
    libxkbfile1 libsecret-1-0
service docker start
docker info
docker run --rm ubuntu:24.04 true

# Electron runs as an ordinary user, with its sandbox enabled.
useradd --create-home --shell /bin/bash prysma
usermod -aG docker prysma
install -d /opt/prysma-vscode
curl --fail --location --retry 3 \
    https://update.code.visualstudio.com/1.96.4/linux-x64/stable -o /tmp/prysma-vscode.tar.gz
tar -xzf /tmp/prysma-vscode.tar.gz -C /opt/prysma-vscode --strip-components=1
chmod 4755 /opt/prysma-vscode/chrome-sandbox
python3 -m venv /opt/prysma-tests
/opt/prysma-tests/bin/pip install -r "$1/tests/environnement_dev_tests/requirements.txt"

# Keep executable modes and Linux permission tests off the Windows-mounted drive.
install -d -o prysma -g prysma /home/prysma/workspace
tar -C "$1" --exclude=.git --exclude=build --exclude=.venv --exclude=venv \
    --exclude=node_modules --exclude=__pycache__ -cf - . | tar -C /home/prysma/workspace -xf -
chown -R prysma:prysma /home/prysma/workspace
chmod +x /home/prysma/workspace/docker/environnement_dev/linux/*.sh
