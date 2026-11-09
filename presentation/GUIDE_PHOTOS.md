# Guide des images de la présentation

14 images à mettre dans `presentation/images/procgen/`, **en .png, avec exactement ces noms**.
Tant qu'une image manque, la diapo affiche un cadre en pointillés avec le nom attendu.

## Comment capturer

- **F12** = capture de la **carte seule** (le plus propre pour les diapos)
- **F11** = capture de **toute la fenêtre** (panneau ImGui compris)
- Les fichiers arrivent dans `captures/`, **à côté de l'exécutable**
  (en général `cmake-build-debug/captures/`), nommés `demo_seedXXXX.png`.
  Il suffit de les **renommer** et de les **déplacer** dans `presentation/images/procgen/`.
- Toutes les démos démarrent avec la **seed 1234** : gardez-la pour retrouver les mêmes images.
- Pas besoin de respecter un format exact : l'image s'adapte au cadre sans être déformée.
  Les cartes sont en 4:3, la fenêtre en 16:9.

---

## 1. `exemples_jeux.png` (diapo 2 : « C'est quoi ? »)

**Pas une capture du programme.** Un bandeau très large (environ 4 × 1) avec 3 ou 4 jeux
qui utilisent la génération procédurale, côte à côte. Par exemple :
*Minecraft* (terrain), *Spelunky* (niveaux), *No Man's Sky* (planètes), *Dead Cells* (salles assemblées).

> Prenez les images dans les **kits presse officiels** des jeux, ou faites vos propres captures.

## 2. `markov_demo.png` (diapo 11 : « La démo Markov »)

- Démo **0. Chaîne de Markov**, **F11** (toute la fenêtre)
- Liste : **villes**, Ordre : **2**
- Dans « Qu'est-ce qui suit... » tapez **`o`** pour voir les barres de probabilités

À montrer : les noms inventés en jaune, quelques copies en gris, et la table du bas.

## 3. `perlin_carte.png` (diapo 21 : « Du bruit à la carte »)

- Démo **1. Perlin Noise**, **F12** (carte seule)
- Type : **Perlin**, Affichage : **Terrain (seuils)**, Octaves : **6**
- **Étirer entre 0 et 1 : coché**
- **Île : cochée** (force 0.6)

À montrer : une belle île avec toutes les couleurs, de l'eau profonde jusqu'à la neige.

## 4. `perlin_histogramme.png` (diapo 22 : « Piège : le fBm reste au milieu »)

- Démo **1. Perlin Noise**, **F11** (toute la fenêtre : il faut l'histogramme du panneau)
- Type : **Perlin**, Affichage : **Terrain (seuils)**, Octaves : **6**, Île : **décochée**
- **Étirer entre 0 et 1 : DÉCOCHÉ**

À montrer : l'histogramme tout serré au milieu, et une carte sans eau profonde ni neige
(seulement eau claire, sable et herbe). Vous pouvez recadrer pour garder l'histogramme lisible.

## 5. `drunkard_brut.png` (diapo 25 : « Drunkard's Walk : résultat »)

On veut montrer **les défauts** de la version de base :

- Démo **2. Drunkard + automate**, mode **Drunkard's Walk**
- À creuser : **0.40**, Ivrognes : **1**, Pinceau : **1**
- **Tout droit : 0**, **Durée de vie : 0**
- Attendez que l'ivrogne ait fini de creuser (la barre disparaît), **sans** passe d'automate, puis **F12**

À montrer : la grosse tache autour du départ, les bords en dents de scie.

## 6. `automate_seul.png` (diapo 30 : « L'automate tout seul ? »)

- Démo **2**, mode **Automate seul (au hasard)**, Murs au départ : **0.50**
- Appuyez **5 fois sur Espace** (5 passes)
- **Colorer les zones : coché**
- **F12**

À montrer : de belles grottes, mais une couleur par zone séparée (19 zones avec la seed 1234).

## 7 à 9. Le combo (diapo 31) : **3 captures à la suite, même seed**

Démo **2**, mode **Drunkard's Walk**, réglages **par défaut** :
à creuser 0.35, ivrognes 1, **pinceau 1**, tout droit 0.30, durée de vie 200.
Cochez **Colorer les zones** dès le début.

| Image | Quand capturer (F12) |
|---|---|
| `combo_1_drunkard.png` | Quand l'ivrogne a fini de creuser |
| `combo_2_automate.png` | Après **4 fois Espace** : on doit voir **plusieurs couleurs** (zones coupées) |
| `combo_3_nettoyage.png` | Après **« 3. Nettoyer (flood fill) »** : une seule couleur |

> Si la seed 1234 ne coupe pas la grotte à l'étape 2 (« Tout est relié ! » en vert),
> appuyez sur **R** et recommencez jusqu'à voir « Zones séparées » en rouge.

## 10. `bsp_resultat.png` (diapo 37 : « BSP : résultat »)

- Démo **3. BSP**, réglages par défaut
- Découpes, Salles et Couloirs **cochés**, Profondeur montrée **au maximum**
- **F12**

À montrer : les lignes de découpe colorées par profondeur, les salles et les couloirs.

> Bonus si vous voulez un GIF ou plusieurs images : **Espace** montre la découpe profondeur par profondeur.

## 11 et 12. Le WFC (diapo 43) : **2 captures, même seed**

Démo **4. Wave Function Collapse**, poids et règles **par défaut**.

| Image | Réglages | Puis |
|---|---|---|
| `wfc_sans_guide.png` | Guider avec un Perlin : **décoché** | « Tout finir », **F12** |
| `wfc_guide.png` | Guider avec un Perlin : **coché**, Force **20** | « Tout finir », **F12** |

À montrer : à gauche un mélange « bruité », à droite de vraies îles avec des plages autour.

> Bonus pour la diapo « WFC : la propagation » : une capture **en cours** (cochez « En marche »
> avec une vitesse de 1, et faites F12 à mi-chemin) montre les cases sombres encore « en superposition ».

## 13. `unity_rule_tile.png` (diapo 45 : « Dans Unity : les Rule Tiles »)

**Capture de Unity**, pas du programme. Format vertical (environ 420 × 470).

- Package **2D Tilemap Extras** installé
- *Create → 2D → Tiles → Rule Tile*
- Ajoutez 3 ou 4 règles (coin, bord, milieu) avec leurs sprites
- Capturez l'**Inspector** de la Rule Tile, avec les grilles 3×3 (flèches vertes, croix rouges) bien visibles

## 14. `projet_fenetre.png` (diapo 50 : « Le projet C++ / SFML »)

- N'importe quelle démo, **F11** (toute la fenêtre)
- Conseil : le **WFC en cours** de génération avec le guide, ou le **Perlin** en terrain

À montrer : le panneau ImGui à gauche et la carte à droite.
