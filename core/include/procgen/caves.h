#ifndef CORE_PROCGEN_CAVES_H
#define CORE_PROCGEN_CAVES_H

#include <cstdint>
#include <vector>

#include "procgen/grid.h"
#include "procgen/random.h"

namespace core::procgen {

// ---------------------------------------------------------------------------
// Drunkard's Walk : un "ivrogne" part du centre d'une carte pleine de murs
// et creuse en marchant au hasard, jusqu'a avoir creuse assez de cases.
// ---------------------------------------------------------------------------
struct DrunkardSettings {
  int width = 120;
  int height = 90;
  float fill_ratio = 0.40f;  // part de la carte a creuser (0.40 = 40 %)
  int walkers = 1;           // nombre d'ivrognes
  int brush = 1;             // taille du "pinceau" (1 = une case)
  float straight_bias = 0.0f;  // chance de garder la meme direction
  int lifetime = 0;  // pas avant de "reapparaitre" sur une case deja creusee (0 = jamais)
};

class DrunkardWalk {
 public:
  void Start(const DrunkardSettings& settings, std::uint32_t seed);

  // Fait faire UN pas a chaque ivrogne. Renvoie false quand c'est fini.
  bool Step();

  // Fait tous les pas d'un coup.
  void Finish();

  [[nodiscard]] bool finished() const { return dug_ >= target_ || stuck_; }
  [[nodiscard]] const Grid<Cell>& grid() const { return grid_; }
  [[nodiscard]] const std::vector<Point>& walkers() const { return walkers_; }
  [[nodiscard]] int dug() const { return dug_; }
  [[nodiscard]] int target() const { return target_; }
  [[nodiscard]] long long steps() const { return steps_; }

 private:
  void Dig(int cx, int cy);
  Point RandomFloorCell();

  DrunkardSettings settings_;
  Random random_;
  Grid<Cell> grid_;
  std::vector<Point> walkers_;
  std::vector<int> directions_;  // derniere direction de chaque ivrogne
  std::vector<int> ages_;        // nombre de pas depuis la derniere apparition
  int dug_ = 0;
  int target_ = 0;
  long long steps_ = 0;
  bool stuck_ = false;
};

// ---------------------------------------------------------------------------
// Automate cellulaire : chaque case regarde ses 8 voisines.
//   plus de "threshold" murs autour  -> mur
//   moins de "threshold" murs autour -> sol
//   exactement "threshold"           -> on ne change rien
// ---------------------------------------------------------------------------

// Remplit la carte au hasard (bords = murs). wall_ratio = part de murs.
void RandomFill(Grid<Cell>& grid, Random& random, float wall_ratio);

// Nombre de murs parmi les 8 voisines (une case hors de la carte = un mur).
int CountWallNeighbours(const Grid<Cell>& grid, int x, int y);

// Une passe d'automate. Renvoie une NOUVELLE grille : on ne modifie jamais
// la grille qu'on est en train de lire.
Grid<Cell> CaveStep(const Grid<Cell>& grid, int threshold = 4);

// ---------------------------------------------------------------------------
// Flood fill : trouver les zones de sol reliees entre elles.
// ---------------------------------------------------------------------------
struct Regions {
  Grid<int> labels;        // numero de zone de chaque case (-1 = mur)
  std::vector<int> sizes;  // nombre de cases de chaque zone
};

// Deplacement en 4 directions (comme le joueur) : les diagonales ne relient pas.
Regions FindRegions(const Grid<Cell>& grid);

// Garde la plus grande zone, rebouche toutes les autres.
// Renvoie le nombre de zones supprimees.
int KeepLargestRegion(Grid<Cell>& grid);

}  // namespace core::procgen

#endif  // CORE_PROCGEN_CAVES_H
