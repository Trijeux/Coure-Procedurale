#ifndef SRC_DEMOS_PERLIN_DEMO_H
#define SRC_DEMOS_PERLIN_DEMO_H

#include <array>
#include <vector>

#include "demo.h"
#include "grid_view.h"
#include "procgen/noise.h"

namespace src::demo {

// 1. Perlin Noise : du bruit blanc au terrain.
class PerlinDemo final : public Demo {
 public:
  PerlinDemo();

  [[nodiscard]] const char* name() const override { return "1. Perlin Noise"; }
  [[nodiscard]] const char* file_tag() const override { return "perlin"; }

  void Gui() override;
  void Draw(sf::RenderTarget& target) override;

 protected:
  void Regenerate() override;

 private:
  static constexpr int kWidth = 480;   // 1 case = 2x2 pixels
  static constexpr int kHeight = 360;

  core::procgen::Noise noise_;
  view::GridView view_;
  std::vector<float> heights_;
  std::array<float, 40> histogram_{};

  int type_ = 2;     // 0 = blanc, 1 = value, 2 = perlin
  int display_ = 1;  // 0 = gris, 1 = terrain
  float zoom_ = 0.012f;
  int octaves_ = 6;
  float persistence_ = 0.5f;
  float lacunarity_ = 2.0f;
  float offset_[2] = {0.0f, 0.0f};
  bool normalize_ = true;
  bool island_ = false;
  float island_strength_ = 0.6f;
  float sea_level_ = 0.0f;
};

}  // namespace src::demo

#endif  // SRC_DEMOS_PERLIN_DEMO_H
