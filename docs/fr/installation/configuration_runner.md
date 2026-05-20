# Configuration du Runner GitHub Actions

Ce document décrit comment installer, configurer et exécuter le runner auto-hébergé (Self-Hosted Runner) GitHub Actions sur le serveur Debian 12 afin d'automatiser les tests de performance de Prysma.

## 1. Prérequis sur la plateforme GitHub

1. Allez sur votre dépôt de code sur **GitHub**.
2. Cliquez sur l'onglet **Settings** (Paramètres) en haut.
3. Dans le menu latéral de gauche, cliquez sur **Actions** puis sur **Runners**.
4. Cliquez sur le bouton vert **New self-hosted runner** (Nouveau runner auto-hébergé).
5. Sélectionnez la plateforme : **Linux** et l'architecture **x64**.
6. Conservez cette page ouverte, elle contient les liens de téléchargement et le jeton d'enregistrement unique.

## 2. Installation sur le serveur OptiPlex

Connectez-vous en SSH à votre serveur et exécutez les commandes suivantes dans votre répertoire utilisateur :

```bash
# 1. Créez un dossier dédié au runner et allez-y
mkdir ~/actions-runner && cd ~/actions-runner

# 2. Téléchargez l'archive officielle du runner (remplacez XXXX par la version actuelle indiquée par GitHub)
curl -o actions-runner-linux-x64-XXXX.tar.gz -L https://github.com/actions/runner/releases/download/vXXXX/actions-runner-linux-x64-XXXX.tar.gz

# 3. Extrayez l'archive
tar xzf ./actions-runner-linux-x64-XXXX.tar.gz
```

## 3. Configuration et Enregistrement

Exécutez le script de configuration en remplaçant l'URL et le token par ceux affichés sur la page GitHub de l'Étape 1 :

```bash
./config.sh --url https://github.com/VOTRE_PSEUDO/VOTRE_DEPOT --token VOTRE_TOKEN_UNIQUE
```

*Lors de la configuration, vous pouvez accepter toutes les options proposées par défaut en appuyant sur la touche **Entrée**.*

## 4. Exécution permanente (Service Système)

Pour garantir que le runner s'exécute automatiquement en tâche de fond et survive aux redémarrages du serveur :

```bash
# Installez le runner en tant que service systemd
sudo ./svc.sh install

# Démarrez le service
sudo ./svc.sh start

# Vérifiez le statut du service
sudo ./svc.sh status
```

Une fois le service démarré, le runner apparaîtra en vert avec l'état `Idle` sur la plateforme GitHub (onglet **Settings** -> **Actions** -> **Runners**), prêt à recevoir et exécuter vos jobs d'intégration continue.
