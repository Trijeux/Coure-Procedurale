#include "procgen/caves.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <queue>

namespace core::procgen {

namespace {

// Haut, droite, bas, gauche.
constexpr std::array<Point, 4> kDirections = {{{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};

}  // namespace

// ============================== Drunkard's Walk ==============================

void DrunkardWalk::Start(const DrunkardSettings& settings, std::uint32_t seed) {
  settings_ = settings;
  random_.Reseed(seed);
  grid_ = Grid<Cell>(settings.width, settings.height, Cell::kWall);

  // On ne creuse jamais le bord : la carte reste fermee.
  const int inside = (settings.width - 2) * (settings.height - 2);
  target_ = static_cast<int>(static_cast<float>(inside) * settings.fill_ratio);
  dug_ = 0;
  steps_ = 0;
  stuck_ = false;

  const int count = std::max(1, settings.walkers);
  walkers_.assign(static_cast<std::size_t>(count),
                  Point{settings.width / 2, settings.height / 2});
  directions_.assign(static_cast<std::size_t>(count), 0);
  ages_.assign(static_cast<std::size_t>(count), 0);
  Dig(settings.width / 2, settings.height / 2);
}

void DrunkardWalk::Dig(int cx, int cy) {
  // Le pinceau : un carre de brush x brush cases autour de l'ivrogne.
  const int half = (settings_.brush - 1) / 2;
  for (int y = cy - half; y < cy - half + settings_.brush; y++) {
    for (int x = cx - half; x < cx - half + settings_.brush; x++) {
      if (x < 1 || y < 1 || x >= grid_.width() - 1 || y >= grid_.height() - 1) {
        continue;
      }
      if (grid_.At(x, y) == Cell::kWall) {
        grid_.At(x, y) = Cell::kFloor;
        dug_++;
      }
    }
  }
}

bool DrunkardWalk::Step() {
  if (finished()) {
    return false;
  }
  for (std::size_t i = 0; i < walkers_.size(); i++) {
    // Soit on garde la meme direction, soit on en tire une nouvelle.
    if (!random_.Chance(settings_.straight_bias)) {
      directions_[i] = random_.Int(0, 3);
    }
    const Point d = kDirections[static_cast<std::size_t>(directions_[i])];
    Point& p = walkers_[i];
    const int nx = p.x + d.x;
    const int ny = p.y + d.y;
    // Contre le bord, on ne bouge pas : il tirera une autre direction au
    // prochain pas. (Le "coller" au bord donne des grottes toutes plates.)
    if (nx < 1 || ny < 1 || nx > grid_.width() - 2 || ny > grid_.height() - 2) {
      directions_[i] = random_.Int(0, 3);
      continue;
    }
    p = {nx, ny};
    Dig(p.x, p.y);

    // Duree de vie : l'ivrogne "reapparait" sur une case deja creusee.
    // Sans ca, il tourne en rond autour du depart et creuse une grosse tache.
    ages_[i]++;
    if (settings_.lifetime > 0 && ages_[i] >= settings_.lifetime) {
      p = RandomFloorCell();
      ages_[i] = 0;
    }
  }
  steps_++;
  // Securite : si on demande de tout creuser, l'ivrogne pourrait tourner
  // tres longtemps pour trouver les dernieres cases.
  if (steps_ > static_cast<long long>(grid_.width()) * grid_.height() * 200) {
    stuck_ = true;
  }
  return !finished();
}

Point DrunkardWalk::RandomFloorCell() {
  // Il y a toujours au moins une case creusee (le depart).
  while (true) {
    const int x = random_.Int(1, grid_.width() - 2);
    const int y = random_.Int(1, grid_.height() - 2);
    if (grid_.At(x, y) == Cell::kFloor) {
      return {x, y};
    }
  }
}

void DrunkardWalk::Finish() {
  while (Step()) {
  }
}

// =========================== Automate cellulaire ============================

void RandomFill(Grid<Cell>& grid, Random& random, float wall_ratio) {
  for (int y = 0; y < grid.height(); y++) {
    for (int x = 0; x < grid.width(); x++) {
      const bool border =
          x == 0 || y == 0 || x == grid.width() - 1 || y == grid.height() - 1;
      grid.At(x, y) =
          (border || random.Chance(wall_ratio)) ? Cell::kWall : Cell::kFloor;
    }
  }
}

int CountWallNeighbours(const Grid<Cell>& grid, int x, int y) {
  int walls = 0;
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx == 0 && dy == 0) {
        continue;  // pas la case elle-meme
      }
      const int nx = x + dx;
      const int ny = y + dy;
      if (!grid.InBounds(nx, ny) || grid.At(nx, ny) == Cell::kWall) {
        walls++;
      }
    }
  }
  return walls;
}

Grid<Cell> CaveStep(const Grid<Cell>& grid, int threshold) {
  Grid<Cell> next = grid;  // la 2e grille : on ecrit ici, on lit dans "grid"
  for (int y = 0; y < grid.height(); y++) {
    for (int x = 0; x < grid.width(); x++) {
      const int walls = CountWallNeighbours(grid, x, y);
      if (walls > threshold) {
        next.At(x, y) = Cell::kWall;
      } else if (walls < threshold) {
        next.At(x, y) = Cell::kFloor;
      }
    }
  }
  // Le bord reste toujours un mur.
  for (int x = 0; x < next.width(); x++) {
    next.At(x, 0) = Cell::kWall;
    next.At(x, next.height() - 1) = Cell::kWall;
  }
  for (int y = 0; y < next.height(); y++) {
    next.At(0, y) = Cell::kWall;
    next.At(next.width() - 1, y) = Cell::kWall;
  }
  return next;
}

// ================================ Flood fill ================================

Regions FindRegions(const Grid<Cell>& grid) {
  Regions regions;
  regions.labels = Grid<int>(grid.width(), grid.height(), -1);

  for (int sy = 0; sy < grid.height(); sy++) {
    for (int sx = 0; sx < grid.width(); sx++) {
      if (grid.At(sx, sy) != Cell::kFloor || regions.labels.At(sx, sy) != -1) {
        continue;  // mur, ou case deja "peinte"
      }
      // Nouvelle zone : on peint tout ce qui est relie (comme le pot de peinture).
      const int id = static_cast<int>(regions.sizes.size());
      int size = 0;
      std::queue<Point> todo;
      todo.push({sx, sy});
      regions.labels.At(sx, sy) = id;
      while (!todo.empty()) {
        const Point p = todo.front();
        todo.pop();
        size++;
        for (const Point& d : kDirections) {
          const int nx = p.x + d.x;
          const int ny = p.y + d.y;
          if (grid.InBounds(nx, ny) && grid.At(nx, ny) == Cell::kFloor &&
              regions.labels.At(nx, ny) == -1) {
            regions.labels.At(nx, ny) = id;
            todo.push({nx, ny});
          }
        }
      }
      regions.sizes.push_back(size);
    }
  }
  return regions;
}

int KeepLargestRegion(Grid<Cell>& grid) {
  const Regions regions = FindRegions(grid);
  if (regions.sizes.size() <= 1) {
    return 0;
  }
  const auto biggest = std::max_element(regions.sizes.begin(), regions.sizes.end());
  const int keep = static_cast<int>(biggest - regions.sizes.begin());

  for (int y = 0; y < grid.height(); y++) {
    for (int x = 0; x < grid.width(); x++) {
      const int label = regions.labels.At(x, y);
      if (label != -1 && label != keep) {
        grid.At(x, y) = Cell::kWall;
      }
    }
  }
  return static_cast<int>(regions.sizes.size()) - 1;
}

}  // namespace core::procgen
