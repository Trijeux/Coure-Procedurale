# Génération procédurale

Démos du cours de génération procédurale, en C++ / SFML 3 / ImGui.
Chaque algorithme est codé à la main, sans bibliothèque de bruit ou de génération.

## Compiler

- CLion + vcpkg (le `vcpkg.json` installe `sfml` et `imgui`)
- Cible : `GenerationProcedurale`
- Le programme doit être lancé depuis le dossier de build : les ressources
  (`resources/markov/*.txt`) y sont copiées automatiquement.

## Organisation

| Dossier | Contenu |
|---|---|
| `core/` | **Les algorithmes**, en C++ pur, sans SFML. C'est la partie à lire pour le cours. |
| `src/`  | L'affichage SFML et les panneaux ImGui de chaque démo. |
| `api/`  | Le gestionnaire de ressources du template, et `external/imgui-sfml` (le lien ImGui ↔ SFML, compilé dans le projet). |
| `resources/markov/` | Les listes de mots pour la chaîne de Markov (un mot par ligne). Ajoutez un `.txt`, il apparaît dans la démo. |
| `presentation/` | La présentation HTML du cours et ses images. |

| Algorithme | Fichier |
|---|---|
| Hasard avec seed (identique sur tous les PC) | `core/include/procgen/random.h` |
| Chaîne de Markov | `core/src/markov.cc` |
| Bruit blanc, Value Noise, Perlin, fBm | `core/src/noise.cc` |
| Drunkard's Walk, automate cellulaire, flood fill | `core/src/caves.cc` |
| BSP | `core/src/bsp.cc` |
| Wave Function Collapse | `core/src/wfc.cc` |

## Touches

| Touche | Action |
|---|---|
| `1` à `5` | Changer de démo |
| `R` | Nouvelle seed |
| `Espace` | Étape suivante (passe d'automate, profondeur BSP, étape WFC, nouveaux noms) |
| Clic gauche | WFC : poser une tuile à la main |
| `F12` | Capturer la carte dans `captures/` |
| `F11` | Capturer toute la fenêtre dans `captures/` |
| `Échap` | Quitter |
