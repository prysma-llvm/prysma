# Site de documentation Prysma / Prysma documentation site

[Documentation officielle / Official documentation](https://prysma-llvm.github.io/prysma/)

GitHub Pages génère une page HTML par README ou guide Markdown avec `website/build.mjs`.
Les fichiers Markdown restent la seule source. Le workflow republie le site et
son sitemap lorsque les docs changent sur `main`. Les anciens liens avec `#fr/`
ou `#en/` redirigent vers les nouvelles adresses.

GitHub Pages publishes HTML directly from the Markdown on `main`. Each page
has its own address and works without JavaScript. To preview the published
version locally (Node.js 24.15+):

```sh
cd website
npm ci --ignore-scripts
npm test
npm run build
python3 -m http.server 8765 --bind 127.0.0.1 --directory dist
```

Pour lire la dernière version de GitHub en direct sans générer le site, servez
`website/` au lieu de `website/dist/`.

## Organiser la navigation / Organize navigation

Ajoutez le fichier **`docs/site.nav` sur GitHub, branche `main`**. Il est lu au prochain déploiement automatique. Il est facultatif : les pages qui ne sont pas
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
par numéro dans le journal du build. A missing navigation file never hides a page.

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
