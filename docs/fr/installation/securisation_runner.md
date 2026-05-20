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

## 2. configurer le serveur

Exécuter ces étapes sur le serveur (en tant que `root` ou avec `sudo` depuis votre compte administrateur) :

### A. Création de l'utilisateur dédié
```bash
# 1. Créer l'utilisateur isolé (sans mot de passe)
sudo adduser --disabled-password --gecos "" actions-runner

# 2. L'ajouter au groupe docker pour lui permettre de lancer les conteneurs de test
sudo usermod -aG docker actions-runner
```

### B. Configuration du Safe Shell
```bash
# 1. Rendre le script exécutable
sudo chmod +x /usr/local/bin/prysma-safe-shell

# 2. Enregistrer le script dans la liste des shells système autorisés
sudo sh -c 'echo "/usr/local/bin/prysma-safe-shell" >> /etc/shells'

# 3. Définir le safe-shell comme shell par défaut pour l'utilisateur
sudo chsh -s /usr/local/bin/prysma-safe-shell actions-runner
```

### C. Déploiement et droits d'accès
Si le runner a déjà été installé dans un autre répertoire (par exemple `/home/zyph/actions-runner`), déplacez-le dans le dossier personnel de l'utilisateur dédié pour éviter les erreurs de droits d'accès au dossier parent (`CHDIR permission denied`) :
```bash
# 1. Aller dans l'ancien dossier et désinstaller le service existant
cd /home/zyph/actions-runner
sudo ./svc.sh stop || true
sudo ./svc.sh uninstall || true

# 2. Déplacer le dossier vers sa nouvelle maison
sudo mv /home/zyph/actions-runner /home/actions-runner/

# 3. Attribuer la propriété des fichiers à l'utilisateur actions-runner
sudo chown -R actions-runner:actions-runner /home/actions-runner/actions-runner
```

### D. Enregistrement du service
```bash
# 1. Aller dans le nouveau dossier
cd /home/actions-runner/actions-runner

# 2. Installer le service systemd pour qu'il s'exécute sous l'utilisateur sécurisé
sudo ./svc.sh install actions-runner

# 3. Démarrer le service
sudo ./svc.sh start
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
