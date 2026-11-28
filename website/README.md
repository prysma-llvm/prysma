# Site de documentation Prysma / Prysma documentation site

Site statique bilingue. Aucun Markdown du projet n’est copié dans le site.
Le navigateur découvre les README et les fichiers `.md` sous `docs/fr` et `docs/en`
sur GitHub `main`, puis charge la page demandée. L’index et les fichiers sont
lus au même commit pour éviter de mélanger deux versions. Le lien « Voir la
source » ouvre le fichier sur `main`.

Static and bilingual. The browser indexes GitHub `main` and fetches only the
requested article. No bundled Markdown, token, build, or database. If GitHub
is unavailable, the reader offers a direct link; already opened documents may
remain in memory for the current tab. The open tab rechecks `main` every five
minutes and when it becomes active, with a five-minute cooldown.

```sh
cd website
python3 -m http.server 8765 --bind 127.0.0.1
# http://127.0.0.1:8765
node --test
```

## Organiser la navigation / Organize navigation

Ajoutez le fichier **`docs/site.nav` sur GitHub, branche `main`**. Le site le
détecte sans redéploiement. Il est facultatif : les pages qui ne sont pas
mentionnées restent dans les catégories automatiques.

Add **`docs/site.nav` to GitHub `main`**. Sections and pages are displayed in
declaration order. Paths are relative to the repository root. The optional
label after `|` changes the sidebar only. Use `#` for a full-line comment.

```text
# docs/site.nav
section fr debut | Prise en main
page README.FR.md | Présentation
page docs/fr/installation/docker_compilation.md | Compiler avec Docker
page docs/fr/language_specification.md | Référence

section en start | Getting started
page README.md | Overview
page docs/en/installation/docker_compilation.md | Build with Docker
page docs/en/prysma.md | Language reference
```

Un chemin doit exister et correspondre à la langue de la section. Une page ne
peut figurer qu’une fois. Les lignes invalides sont ignorées et signalées
par numéro dans la barre latérale. A missing navigation file never hides a page.

## Structurer une page / Structure an article

Les titres Markdown `##` et `###` alimentent le sommaire. Pour les encadrés,
utilisez les marqueurs GitHub, qui restent lisibles également sur GitHub :

```markdown
## Compilation

> [!NOTE]
> Le compilateur cible LLVM 18.

> [!WARNING]
> Utilisez la cible Docker debug pour les points d’arrêt.
```

Supported alerts: `NOTE`, `TIP`, `IMPORTANT`, `WARNING`, `CAUTION`. Labels
are translated by the site, and the body remains ordinary Markdown.

## Code

- `assets/source.mjs`: découverte GitHub, fichiers et liens.
- `assets/navigation.mjs`: analyse du fichier facultatif `docs/site.nav`.
- `assets/render.mjs`: rendu Markdown, assainissement, encadrés et sommaire.
- `assets/app.mjs`: interface et actualisation.
- `assets/vendor/`: Marked 18.3.0 et DOMPurify 3.5.0 avec leurs licences.

Pour publier, servez `index.html` et `assets/` depuis un hébergement statique.
Aucune modification des workflows du dépôt n’est nécessaire pour la lecture
ou les mises à jour de la documentation.
