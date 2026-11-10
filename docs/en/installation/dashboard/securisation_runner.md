# GitHub Actions Runner Security (Safe Shell)

This document details the configuration of a restricted execution shell (`prysma-safe-shell`) on the Debian 12 continuous integration server. This security mechanism prevents unauthorized command execution during GitHub Actions workflows triggered by external contributions (Pull Requests).

The safe shell replaces the default runner shell with a Python script that validates every command against a strict regular expression whitelist. Any command not explicitly permitted is blocked with an exit status of 127.

## 1. Command Filtering Script (`prysma-safe-shell`)

Create the file `/usr/local/bin/prysma-safe-shell` with administrative privileges:

```python
#!/usr/bin/env python3
import sys
import os
import subprocess
import re

# Whitelisted command patterns for the CI runner
ALLOWED_PATTERNS = [
    r'^cp docker/compiler/Dockerfile (\$HOME|.+)/prysma/Dockerfile$',
    r'^cp docker/compiler/entrypoint.sh (\$HOME|.+)/prysma/entrypoint.sh$',
    r'^docker build --target benchmark -t prysma-compiler -f docker/compiler/Dockerfile \.$',
    r'^docker run --rm --privileged --pid=host -v /var/run/docker\.sock:/var/run/docker\.sock optiplex-lab-mode enable$',
    r'^docker run --rm --cap-add=SYS_ADMIN --cpuset-cpus="1-3" -v "(\$HOME|.+)/prysma":/prysma -v ".+":/workspace prysma-compiler ".+"$',
    r'^python3 tests/save_perf_results\.py$',
    r'^docker run --rm --privileged --pid=host -v /var/run/docker\.sock:/var/run/docker\.sock optiplex-lab-mode disable$',
    r'^docker run --rm -v ".+":/workspace alpine chown -R \d+:\d+ /workspace$'
]

def main():
    script_path = None
    for arg in sys.argv:
        if arg.endswith('.sh') and os.path.exists(arg):
            script_path = arg
            break
            
    if not script_path:
        if '-c' in sys.argv:
            idx = sys.argv.index('-c')
            cmd_to_run = sys.argv[idx+1].strip()
            if cmd_to_run.startswith('echo') or cmd_to_run == 'locale':
                return subprocess.run(['/bin/bash'] + sys.argv[1:]).returncode
            print(f"Blocked unauthorized command: {cmd_to_run}", file=sys.stderr)
            return 127
        return subprocess.run(['/bin/bash'] + sys.argv[1:]).returncode

    with open(script_path, 'r') as f:
        content = f.read().strip()

    # Reconstruct lines with backslash continuations
    raw_lines = content.splitlines()
    lines = []
    current_line = ""
    for line in raw_lines:
        line = line.strip()
        if not line or line.startswith('#') or line.startswith('set -e') or line.startswith('set -o'):
            continue
        if line.endswith('\\'):
            current_line += line[:-1].strip() + " "
        else:
            current_line += line
            lines.append(current_line.strip())
            current_line = ""
    if current_line:
        lines.append(current_line.strip())

    for line in lines:
        matched = False
        for pattern in ALLOWED_PATTERNS:
            if re.match(pattern, line):
                matched = True
                break
        if not matched:
            print(f"Blocked unauthorized command: {line}", file=sys.stderr)
            return 127

    return subprocess.run(['/bin/bash'] + sys.argv[1:]).returncode

if __name__ == '__main__':
    sys.exit(main())
```

## 2. Host Server Configuration

Execute the following steps on the host server with `sudo` privileges:

### A. Dedicated User Account Creation
```bash
# Create a system user without password login
sudo adduser --disabled-password --gecos "" actions-runner

# Grant Docker group access to run test containers
sudo usermod -aG docker actions-runner
```

### B. Safe Shell Registration
```bash
# Make the script executable
sudo chmod +x /usr/local/bin/prysma-safe-shell

# Register the script in the valid system shells list
sudo sh -c 'echo "/usr/local/bin/prysma-safe-shell" >> /etc/shells'

# Assign the safe shell as default for actions-runner
sudo chsh -s /usr/local/bin/prysma-safe-shell actions-runner
```

### C. Runner Service Deployment
```bash
# Move the runner to the dedicated user directory
sudo mv /home/zyph/actions-runner /home/actions-runner/
sudo chown -R actions-runner:actions-runner /home/actions-runner/actions-runner

# Install and start the systemd service under the actions-runner account
cd /home/actions-runner/actions-runner
sudo ./svc.sh install actions-runner
sudo ./svc.sh start
```

### D. Database Access Permissions (ACLs)
Grant the `actions-runner` account access to write metrics into the dashboard SQLite database without elevating system privileges:

```bash
sudo apt-get install -y acl
sudo setfacl -m u:actions-runner:x /home/zyph
sudo setfacl -R -m u:actions-runner:rwx /home/zyph/dashboard-prysma
sudo setfacl -R -d -m u:actions-runner:rwx /home/zyph/dashboard-prysma
```

## 3. Filtering Validation

When an instruction outside the whitelist is executed within the CI workflow, the runner intercepts the execution:

```text
Blocked unauthorized command: cat /etc/passwd && rm -rf /home/zyph/dashboard-prysma
Error: Process completed with exit code 127.
```

The GitHub Actions job terminates immediately with exit code 127, preserving host security.
