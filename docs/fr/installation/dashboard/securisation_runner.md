# Sécurisation du Runner GitHub Actions (Safe Shell)

Ce document décrit la configuration d'un shell de restriction (`prysma-safe-shell`) sur le serveur d'intégration continue Debian 12. Ce mécanisme empêche l'exécution de commandes non autorisées lors de l'exécution de workflows GitHub Actions déclenchés par des contributions externes (Pull Requests).

Le principe consiste à remplacer le shell par défaut du runner par un script Python qui valide chaque instruction exécutée contre une liste blanche stricte d'expressions régulières. Toute commande non conforme est interceptée et bloquée avec un code de retour 127.

## 1. Script de Filtrage des Commandes (`prysma-safe-shell`)

Créez le fichier `/usr/local/bin/prysma-safe-shell` avec les privilèges d'administration :

```python
#!/usr/bin/env python3
import sys
import os
import subprocess
import re

# Modèles de commandes autorisées pour le runner
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
            print(f"Commande non autorisée : {cmd_to_run}", file=sys.stderr)
            return 127
        return subprocess.run(['/bin/bash'] + sys.argv[1:]).returncode

    with open(script_path, 'r') as f:
        content = f.read().strip()

    # Reconstitution des lignes avec continuation par barre oblique inverse (\)
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
            print(f"Commande non autorisée : {line}", file=sys.stderr)
            return 127

    return subprocess.run(['/bin/bash'] + sys.argv[1:]).returncode

if __name__ == '__main__':
    sys.exit(main())
```

## 2. Configuration du Serveur Hôte

Exécutez ces étapes sur le serveur hôte avec les privilèges `sudo` :

### A. Création du compte utilisateur dédié
```bash
# Création d'un utilisateur système sans mot de passe
sudo adduser --disabled-password --gecos "" actions-runner

# Attribution des droits Docker pour lancer les conteneurs de test
sudo usermod -aG docker actions-runner
```

### B. Enregistrement du Shell Restreint
```bash
# Définition des permissions d'exécution
sudo chmod +x /usr/local/bin/prysma-safe-shell

# Enregistrement dans les shells valides du système
sudo sh -c 'echo "/usr/local/bin/prysma-safe-shell" >> /etc/shells'

# Attribution comme shell par défaut pour actions-runner
sudo chsh -s /usr/local/bin/prysma-safe-shell actions-runner
```

### C. Déploiement du Service du Runner
```bash
# Déplacement du runner dans le répertoire personnel dédié
sudo mv /home/zyph/actions-runner /home/actions-runner/
sudo chown -R actions-runner:actions-runner /home/actions-runner/actions-runner

# Installation et démarrage du service systemd sous l'utilisateur actions-runner
cd /home/actions-runner/actions-runner
sudo ./svc.sh install actions-runner
sudo ./svc.sh start
```

### D. Gestion des Droits sur la Base de Données (ACLs)
Pour permettre à l'utilisateur `actions-runner` d'enregistrer les métriques de benchmarks dans la base SQLite sans lui accorder de privilèges administratifs :

```bash
sudo apt-get install -y acl
sudo setfacl -m u:actions-runner:x /home/zyph
sudo setfacl -R -m u:actions-runner:rwx /home/zyph/dashboard-prysma
sudo setfacl -R -d -m u:actions-runner:rwx /home/zyph/dashboard-prysma
```

## 3. Validation du Filtrage

Lorsqu'une instruction non présente dans la liste blanche est invoquée dans le workflow CI, le script intercepte l'appel :

```text
Commande non autorisée : cat /etc/passwd && rm -rf /home/zyph/dashboard-prysma
Error: Process completed with exit code 127.
```

Le job GitHub Actions échoue immédiatement sans compromettre le système hôte.
