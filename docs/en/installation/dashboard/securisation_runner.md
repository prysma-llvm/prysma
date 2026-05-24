# securing the runner to block garbage commands

if someone makes a pr and modifies the ci.yml to try to hack the server, this thing blocks everything right away. basically we replace the default bash with a python script that checks that the commands are exactly the ones we want. if they're not, it blocks.

## 1. the python security script

create the file `/usr/local/bin/prysma-safe-shell` as root and copy this:

```python
#!/usr/bin/env python3
import sys
import os
import subprocess
import re

# allowed commands, if it's not there it blocks right away
ALLOWED_PATTERNS = [
    r'^cp docker/server/compiler/Dockerfile (\$HOME|.+)/prysma/Dockerfile$',
    r'^cp docker/server/compiler/entrypoint.sh (\$HOME|.+)/prysma/entrypoint.sh$',
    r'^docker build -t prysma-compiler docker/server/compiler$',
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
            print(f"block command: {cmd_to_run}", file=sys.stderr)
            return 127
        return subprocess.run(['/bin/bash'] + sys.argv[1:]).returncode

    with open(script_path, 'r') as f:
        content = f.read().strip()

    # Combine backslash line continuations
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
            print(f"block command: {line}", file=sys.stderr)
            return 127

    return subprocess.run(['/bin/bash'] + sys.argv[1:]).returncode

if __name__ == '__main__':
    sys.exit(main())
```

## 2. configure the server

Run these steps on the server (as `root` or with `sudo` from your administrator account):

### A. Dedicated User Creation
```bash
# 1. Create the isolated user (no password)
sudo adduser --disabled-password --gecos "" actions-runner

# 2. Add them to the docker group to allow them to launch test containers
sudo usermod -aG docker actions-runner
```

### B. Safe Shell Configuration
```bash
# 1. Make the script executable
sudo chmod +x /usr/local/bin/prysma-safe-shell

# 2. Register the script in the list of authorized system shells
sudo sh -c 'echo "/usr/local/bin/prysma-safe-shell" >> /etc/shells'

# 3. Set the safe-shell as the default shell for the user
sudo chsh -s /usr/local/bin/prysma-safe-shell actions-runner
```

### C. Deployment and Access Rights
If the runner has already been installed in another directory (for example `/home/zyph/actions-runner`), move it to the dedicated user's home folder to avoid access rights errors to the parent directory (`CHDIR permission denied`):
```bash
# 1. Go to the old folder and uninstall the existing service
cd /home/zyph/actions-runner
sudo ./svc.sh stop || true
sudo ./svc.sh uninstall || true

# 2. Move the folder to its new home
sudo mv /home/zyph/actions-runner /home/actions-runner/

# 3. Assign file ownership to the actions-runner user
sudo chown -R actions-runner:actions-runner /home/actions-runner/actions-runner
```

### D. Service Registration
```bash
# 1. Go to the new folder
cd /home/actions-runner/actions-runner

# 2. Install the systemd service so it runs under the secured user
sudo ./svc.sh install actions-runner

# 3. Start the service
sudo ./svc.sh start
```

### E. Dashboard Database Access (SQLite)
To allow the isolated `actions-runner` user to record performance test results in the dashboard's SQLite database (located in the main user's folder `/home/zyph/dashboard-prysma/`), configure access rights via ACLs:
```bash
# 1. Install the acl package on the server
sudo apt-get install acl -y

# 2. Allow actions-runner to traverse the /home/zyph home directory
sudo setfacl -m u:actions-runner:x /home/zyph

# 3. Grant full read/write permissions on the dashboard folder and its files
sudo setfacl -R -m u:actions-runner:rwx /home/zyph/dashboard-prysma
sudo setfacl -R -d -m u:actions-runner:rwx /home/zyph/dashboard-prysma
```

## 3. security test

if in the ci.yml there's a suspicious command like this:

```yaml
run: cat /etc/passwd && rm -rf /home/zyph/dashboard-prysma
```

it will block right away and crash the git step with this:

```text
block command: cat /etc/passwd && rm -rf /home/zyph/dashboard-prysma
Error: Process completed with exit code 127.
```

same thing, impossible to launch another suspicious script (like `sh /tmp/test.sh`), everything is blocked.
