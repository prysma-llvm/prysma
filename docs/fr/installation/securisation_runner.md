# sécuriser le runner pour bloquer les commandes de merde

si quelqu'un fait une pr et modifie le ci.yml pour essayer de hack le serveur, ce truc permet de tout bloquer direct. en gros on remplace le bash par défaut par un script python qui vérifie que les commandes sont exactement celles qu'on veut. si c'est pas le cas, ça bloque.

## 1. le script python de sécurité

créer le fichier `/usr/local/bin/prysma-safe-shell` en root et copier ça :

```python
#!/usr/bin/env python3
import sys
import os
import subprocess
import re

# les commandes autorisées, si c'est pas là ça bloque direct
ALLOWED_PATTERNS = [
    r'^cp docker/server/compiler/Dockerfile (\$HOME|.+)/prysma/Dockerfile$',
    r'^cp docker/server/compiler/entrypoint.sh (\$HOME|.+)/prysma/entrypoint.sh$',
    r'^docker build -t prysma-compiler docker/server/compiler$',
    r'^docker run --rm --privileged --pid=host -v /var/run/docker\.sock:/var/run/docker\.sock optiplex-lab-mode enable$',
    r'^docker run --rm --privileged --cpuset-cpus="1-3" -v "(\$HOME|.+)/prysma":/prysma -v ".+":/workspace prysma-compiler ".+"$',
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

## 2. configurer le serveur

faire ça en root :

```bash
chmod +x /usr/local/bin/prysma-safe-shell
echo "/usr/local/bin/prysma-safe-shell" >> /etc/shells
chsh -s /usr/local/bin/prysma-safe-shell actions-runner
```

## 3. test de sécurité

si dans le ci.yml y'a une commande suspecte comme ça :

```yaml
run: cat /etc/passwd && rm -rf /home/zyph/dashboard-prysma
```

ça va bloquer direct et crasher le step de git avec ça :

```text
block command: cat /etc/passwd && rm -rf /home/zyph/dashboard-prysma
Error: Process completed with exit code 127.
```

pareil, impossible de lancer un autre script suspect (genre `sh /tmp/test.sh`), tout est bloqué.
