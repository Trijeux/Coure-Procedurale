#ifndef CORE_PROCGEN_GRID_H
#define CORE_PROCGEN_GRID_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace core::procgen {

// Une case de carte : mur ou sol.
enum class Cell : std::uint8_t { kWall, kFloor };

// Une position entiere sur une grille.
struct Point {
  int x = 0;
  int y = 0;
};

// Un tableau 2D stocke dans un seul std::vector (case (x, y) = y * largeur + x).
template <typename T>
class Grid {
 public:
  Grid() = default;
  Grid(int width, int height, T value = T{})
      : width_(width),
        height_(height),
        cells_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height),
               value) {}

  [[nodiscard]] int width() const { return width_; }
  [[nodiscard]] int height() const { return height_; }

  [[nodiscard]] bool InBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
  }

  T& At(int x, int y) { return cells_[Index(x, y)]; }
  [[nodiscard]] const T& At(int x, int y) const { return cells_[Index(x, y)]; }

  void Fill(T value) { std::fill(cells_.begin(), cells_.end(), value); }

 private:
  [[nodiscard]] std::size_t Index(int x, int y) const {
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
           static_cast<std::size_t>(x);
  }

  int width_ = 0;
  int height_ = 0;
  std::vector<T> cells_;
};

}  // namespace core::procgen

#endif  // CORE_PROCGEN_GRID_H
