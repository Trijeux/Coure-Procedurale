#ifndef CORE_PROCGEN_BSP_H
#define CORE_PROCGEN_BSP_H

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "procgen/grid.h"
#include "procgen/random.h"

namespace core::procgen {

struct Rect {
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;

  [[nodiscard]] Point Center() const { return {x + w / 2, y + h / 2}; }
};

// Un noeud de l'arbre BSP : une zone, et soit 2 enfants, soit une salle.
struct BspNode {
  Rect area;
  int depth = 0;
  bool vertical_cut = false;  // coupe verticale (gauche | droite) ?
  std::unique_ptr<BspNode> left;
  std::unique_ptr<BspNode> right;
  std::optional<Rect> room;  // seulement dans les feuilles

  [[nodiscard]] bool IsLeaf() const { return left == nullptr; }
};

// Un couloir en L : from -> corner -> to.
struct Corridor {
  Point from;
  Point corner;
  Point to;
  int depth = 0;  // profondeur du noeud qui a cree ce couloir
};

struct BspSettings {
  int width = 96;
  int height = 72;
  int min_leaf = 12;          // taille mini d'une zone (en cases)
  int max_depth = 5;          // nombre de decoupes maximum
  float min_ratio = 0.35f;    // on coupe entre 35 % et 65 % de la zone
  bool split_longest = true;  // couper le cote le plus long (sinon au hasard)
  int min_room = 4;           // taille mini d'une salle
  int room_margin = 1;        // mur minimum entre la salle et le bord de sa zone
  float room_fill = 0.6f;     // taille mini de la salle par rapport a sa zone
};

class Bsp {
 public:
  void Generate(const BspSettings& settings, std::uint32_t seed);

  [[nodiscard]] const BspNode& root() const { return *root_; }
  [[nodiscard]] const std::vector<Corridor>& corridors() const { return corridors_; }
  [[nodiscard]] int leaf_count() const { return leaf_count_; }
  [[nodiscard]] int depth_reached() const { return depth_reached_; }

  // La carte finale : murs partout, sol dans les salles et les couloirs.
  [[nodiscard]] Grid<Cell> Rasterize() const;

 private:
  void Split(BspNode& node);
  void CreateRoom(BspNode& node);
  Point Connect(BspNode& node);  // relie les 2 enfants, renvoie un point de salle

  BspSettings settings_;
  Random random_;
  std::unique_ptr<BspNode> root_;
  std::vector<Corridor> corridors_;
  int leaf_count_ = 0;
  int depth_reached_ = 0;
};

}  // namespace core::procgen

#endif  // CORE_PROCGEN_BSP_H
