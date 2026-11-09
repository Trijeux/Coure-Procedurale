#include "procgen/bsp.h"

#include <algorithm>
#include <cstdlib>

namespace core::procgen {

void Bsp::Generate(const BspSettings& settings, std::uint32_t seed) {
  settings_ = settings;
  random_.Reseed(seed);
  corridors_.clear();
  leaf_count_ = 0;
  depth_reached_ = 0;

  // La racine : toute la carte, moins le bord.
  root_ = std::make_unique<BspNode>();
  root_->area = {1, 1, settings.width - 2, settings.height - 2};

  Split(*root_);    // 1. decouper
  CreateRoom(*root_);  // 2. une salle par feuille
  Connect(*root_);  // 3. relier en remontant l'arbre
}

void Bsp::Split(BspNode& node) {
  depth_reached_ = std::max(depth_reached_, node.depth);
  if (node.depth >= settings_.max_depth) {
    return;  // assez decoupe : c'est une feuille
  }

  const Rect& a = node.area;
  bool vertical = random_.Chance(0.5f);
  if (settings_.split_longest) {
    // Couper le cote le plus long evite les zones toutes fines.
    if (a.w > a.h) {
      vertical = true;
    } else if (a.h > a.w) {
      vertical = false;
    }
  }

  const int size = vertical ? a.w : a.h;
  const int low = std::max(settings_.min_leaf,
                           static_cast<int>(static_cast<float>(size) * settings_.min_ratio));
  const int high = std::min(size - settings_.min_leaf,
                            static_cast<int>(static_cast<float>(size) * (1.0f - settings_.min_ratio)));
  if (low > high) {
    return;  // trop petit pour etre coupe : c'est une feuille
  }
  const int cut = random_.Int(low, high);

  node.vertical_cut = vertical;
  node.left = std::make_unique<BspNode>();
  node.right = std::make_unique<BspNode>();
  if (vertical) {
    node.left->area = {a.x, a.y, cut, a.h};
    node.right->area = {a.x + cut, a.y, a.w - cut, a.h};
  } else {
    node.left->area = {a.x, a.y, a.w, cut};
    node.right->area = {a.x, a.y + cut, a.w, a.h - cut};
  }
  node.left->depth = node.depth + 1;
  node.right->depth = node.depth + 1;

  Split(*node.left);
  Split(*node.right);
}

void Bsp::CreateRoom(BspNode& node) {
  if (!node.IsLeaf()) {
    CreateRoom(*node.left);
    CreateRoom(*node.right);
    return;
  }
  leaf_count_++;

  const Rect& a = node.area;
  const int m = settings_.room_margin;
  const int max_w = a.w - 2 * m;
  const int max_h = a.h - 2 * m;
  if (max_w < 1 || max_h < 1) {
    return;  // zone trop petite pour une salle
  }
  // Une salle entre "room_fill" et 100 % de la place disponible.
  const auto min_size = [this](int max) {
    const int wanted = static_cast<int>(static_cast<float>(max) * settings_.room_fill);
    return std::clamp(std::max(settings_.min_room, wanted), 1, max);
  };
  Rect room;
  room.w = random_.Int(min_size(max_w), max_w);
  room.h = random_.Int(min_size(max_h), max_h);
  room.x = a.x + m + random_.Int(0, max_w - room.w);
  room.y = a.y + m + random_.Int(0, max_h - room.h);
  node.room = room;
}

Point Bsp::Connect(BspNode& node) {
  if (node.IsLeaf()) {
    return node.room ? node.room->Center() : node.area.Center();
  }
  const Point a = Connect(*node.left);
  const Point b = Connect(*node.right);

  // Couloir en L : d'abord en x puis en y, ou l'inverse (au hasard).
  Corridor corridor;
  corridor.from = a;
  corridor.to = b;
  corridor.corner = random_.Chance(0.5f) ? Point{b.x, a.y} : Point{a.x, b.y};
  corridor.depth = node.depth;
  corridors_.push_back(corridor);

  // Le parent se reliera a l'une des deux salles.
  return random_.Chance(0.5f) ? a : b;
}

Grid<Cell> Bsp::Rasterize() const {
  Grid<Cell> grid(settings_.width, settings_.height, Cell::kWall);

  // Les salles (parcours de l'arbre avec une pile).
  std::vector<const BspNode*> todo = {root_.get()};
  while (!todo.empty()) {
    const BspNode* node = todo.back();
    todo.pop_back();
    if (!node->IsLeaf()) {
      todo.push_back(node->left.get());
      todo.push_back(node->right.get());
    } else if (node->room) {
      const Rect& r = *node->room;
      for (int y = r.y; y < r.y + r.h; y++) {
        for (int x = r.x; x < r.x + r.w; x++) {
          grid.At(x, y) = Cell::kFloor;
        }
      }
    }
  }

  // Les couloirs : deux segments droits.
  const auto dig_line = [&grid](Point from, Point to) {
    const int dx = (to.x > from.x) - (to.x < from.x);
    const int dy = (to.y > from.y) - (to.y < from.y);
    Point p = from;
    grid.At(p.x, p.y) = Cell::kFloor;
    while (p.x != to.x || p.y != to.y) {
      p.x += dx;
      p.y += dy;
      grid.At(p.x, p.y) = Cell::kFloor;
    }
  };
  for (const Corridor& c : corridors_) {
    dig_line(c.from, c.corner);
    dig_line(c.corner, c.to);
  }
  return grid;
}

}  // namespace core::procgen
