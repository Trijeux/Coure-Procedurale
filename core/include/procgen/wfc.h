#ifndef CORE_PROCGEN_WFC_H
#define CORE_PROCGEN_WFC_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "procgen/random.h"

namespace core::procgen {

// Wave Function Collapse, version "Simple Tiled Model".
//
// Chaque case garde la LISTE des tuiles encore possibles, sous forme de bits :
// bit 0 = tuile 0, bit 1 = tuile 1... (0b0011 = tuiles 0 et 1 possibles).
//
// A chaque etape :
//   1. Observer   : choisir la case qui a le moins de choix (la plus petite entropie)
//   2. Fixer      : lui donner UNE tuile, tiree au hasard selon les poids
//   3. Propager   : retirer chez les voisines les tuiles qui ne vont plus
class Wfc {
 public:
  using Mask = std::uint32_t;  // jusqu'a 32 tuiles

  enum class Status { kRunning, kDone, kContradiction };

  struct Tile {
    std::string name;
    float weight = 1.0f;  // plus c'est grand, plus la tuile sort souvent
  };

  // Le jeu de tuiles. Efface les regles.
  void SetTiles(std::vector<Tile> tiles);
  void SetWeight(int tile, float weight);

  // Regle de voisinage : a et b ont le droit de se toucher (dans les 2 sens,
  // et dans les 4 directions). Pour des routes ou des tuyaux, il faudrait
  // une regle par direction (voir la presentation : les "bords").
  void Allow(int a, int b);

  // Remet toutes les cases a "tout est possible".
  // "guide" (facultatif) : les tuiles autorisees au depart dans chaque case,
  // par exemple calculees avec un bruit de Perlin. Vide = aucune contrainte.
  void Start(int width, int height, std::uint32_t seed,
             std::vector<Mask> guide = {});

  // Guide "doux" (facultatif) : la tuile preferee de chaque case (-1 = aucune).
  // Au moment de fixer la case, son poids est multiplie par "bias".
  void SetPreferred(std::vector<int> preferred, float bias);

  // Une etape : observer, fixer, propager.
  Status Step();

  // Fixe une case a la main (clic de souris), puis propage.
  // Renvoie false si cette tuile n'est plus possible a cet endroit.
  bool Force(int x, int y, int tile);

  [[nodiscard]] Mask Options(int x, int y) const { return cells_[static_cast<std::size_t>(Index(x, y))]; }
  [[nodiscard]] int OptionCount(int x, int y) const;
  [[nodiscard]] int CollapsedTile(int x, int y) const;  // -1 si pas encore fixee

  [[nodiscard]] const std::vector<Tile>& tiles() const { return tiles_; }
  [[nodiscard]] int width() const { return width_; }
  [[nodiscard]] int height() const { return height_; }
  [[nodiscard]] int collapsed_count() const;
  [[nodiscard]] int contradictions() const { return contradictions_; }
  [[nodiscard]] bool done() const { return done_; }
  [[nodiscard]] bool Allowed(int a, int b) const;
  [[nodiscard]] bool guide_failed() const { return guide_failed_; }

 private:
  [[nodiscard]] int Index(int x, int y) const { return y * width_ + x; }
  [[nodiscard]] float Entropy(Mask mask) const;
  [[nodiscard]] Mask AllTiles() const;
  bool Reset();  // false si le guide lui-meme est impossible

  // Renvoie false en cas de contradiction (une case n'a plus aucun choix).
  bool Propagate(int start);

  std::vector<Tile> tiles_;
  std::vector<Mask> compatible_;  // compatible_[t] = tuiles autorisees a cote de t
  std::vector<Mask> cells_;
  std::vector<Mask> guide_;
  std::vector<int> preferred_;
  float preferred_bias_ = 1.0f;
  Random random_;
  int width_ = 0;
  int height_ = 0;
  int contradictions_ = 0;
  bool done_ = false;
  bool guide_failed_ = false;
};

}  // namespace core::procgen

#endif  // CORE_PROCGEN_WFC_H
