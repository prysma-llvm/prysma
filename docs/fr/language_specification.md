# Spécification du Langage Prysma

## 1. Principes et Syntaxe Générale

Prysma est un langage système compilé ciblant LLVM 18. Il propose une gestion mémoire explicite sans ramasse-miettes, un typage statique strict et une compilation multi-fichiers native.

* **Terminaison des instructions :** Chaque instruction se termine par un point-virgule (`;`).
* **Blocs de code :** Les corps de fonctions, classes et structures de contrôle sont délimités par des accolades `{ }`.
* **Sensibilité à la casse :** Les identifiants respectent strictement la casse.
* **Commentaires :** Les commentaires sur une ligne débutent par `//`.

## 2. Inclusions Multi-Fichiers (`include`)

L'inclusion de modules et de bibliothèques utilise la directive native `include` :

```rust
include "math/geometrie.p";
include "outils/validation.p";
```

Le compilateur résout les chemins relatifs par rapport au fichier courant, déduplique les inclusions et prévient les dépendances cycliques.

## 3. Système de Types

| Type | Taille (bits) | Représentation LLVM IR | Description |
| :--- | :--- | :--- | :--- |
| `int32` | 32 | `i32` | Entier signé standard |
| `int64` | 64 | `i64` | Entier signé 64 bits |
| `float` | 32 | `float` | Nombre à virgule flottante IEEE 754 |
| `bool` | 1 | `i1` | Valeur booléenne (`true`, `false`) |
| `char` | 8 | `i8` | Caractère ASCII |
| `string` | Variable | Tampon mémoire | Chaîne de caractères textuelle |
| `ptr` | 64 | `ptr` | Pointeur mémoire opaque |
| `void` | 0 | `void` | Absence de type (retour de fonction) |
| `T[]` | Variable | Pointeur contigu | Tableau d'éléments de type `T` (ex. `int32[]`) |

## 4. Déclarations et Affectations

* **Déclaration de variable (`dec`) :**
  ```rust
  dec int32 compteur = 0;
  dec float pi = 3.14159;
  dec bool actif = true;
  ```
* **Affectation (`aff`) :**
  Toute modification d'une variable existante requiert le mot-clé `aff` :
  ```rust
  aff compteur = compteur + 1;
  ```

## 5. Fonctions

Les fonctions sont définies avec le mot-clé `fn`. Le passage d'arguments utilise le préfixe `arg`. L'appel de fonction s'effectue obligatoirement avec le mot-clé `call`.

### Définition et appel

```rust
fn int32 calculerSomme(arg int32 a, arg int32 b) {
    return a + b;
}

fn int32 main() {
    dec int32 resultat = call calculerSomme(10, 25);
    return 0;
}
```

### Passage par référence (`ref`) et déréférencement (`unref`)

Pour éviter les copies de mémoire ou transmettre un tampon modifiable :

```rust
fn void afficherMessage(arg string[] texte) {
    call print(ref texte);
}
```

## 6. Structures de Contrôle

### Branchement conditionnel (`if` / `else`)

```rust
if (valeur >= 0) {
    call traiterPositif(valeur);
} else {
    call traiterNegatif(valeur);
}
```

### Boucle `while`

```rust
dec int32 index = 0;
while (index < 10) {
    call afficher(index);
    aff index = index + 1;
}
```

### Boucle `for`

```rust
for (dec int32 i = 0; i < limite; aff i = i + 1) {
    call executer(i);
}
```

### Instruction neutre (`pass`)

L'instruction `pass;` agit comme une opération sans effet (no-op), utile pour expliciter un bloc vide :

```rust
if (condition) {
    pass;
}
```

## 7. Gestion de la Mémoire (Tas)

L'allocation et la libération dynamique sur le tas sont sous le contrôle direct du développeur :

* **`new` :** Alloue un bloc mémoire pour un objet ou un tableau dynamique.
* **`delete` :** Libère immédiatement l'espace mémoire alloué.

```rust
dec ptr instance = new MaClasse();
// Utilisation de l'instance
delete instance;
```

## 8. Modèle Objet (Classes)

Les classes encapsulent des attributs et des méthodes avec des sections d'accès explicites :

```rust
class Entite {
    private:
        dec int32 id;
    public:
        fn void setId(arg int32 nouvelId) {
            aff id = nouvelId;
        }
        fn int32 getId() {
            return id;
        }
}
```

### Héritage

Une classe peut dériver d'une classe parente :

```rust
class Joueur : Entite {
    private:
        dec int32 score;
    public:
        fn void ajouterScore(arg int32 points) {
            aff score = score + points;
        }
}
```

Les sections d'accès supportées sont :
* `public:` : Membres accessibles depuis l'extérieur de la classe.
* `private:` : Membres restreints à la classe courante (visibilité par défaut).
* `protected:` : Membres accessibles à la classe courante et à ses classes dérivées.

## 9. Opérateurs

| Famille | Symboles | Exemple |
| :--- | :--- | :--- |
| **Arithmétique** | `+`, `-`, `*`, `/`, `%` | `a + b * c` |
| **Comparaison** | `==`, `!=`, `<`, `>`, `<=`, `>=` | `if (x != y)` |
| **Logique** | `&&`, `||`, `!` | `if (valide && !bloque)` |
| **Affectation** | `aff` | `aff x = 5;` |
| **Déclaration** | `dec` | `dec int32 x = 5;` |
| **Passage adresse** | `ref` | `call fonction(ref donnee);` |
| **Résolution de portée** | `::` | `Module::methode` |
