#ifndef SRC_DEMOS_WFC_DEMO_H
#define SRC_DEMOS_WFC_DEMO_H

#include <array>
#include <string>

#include "demo.h"
#include "grid_view.h"
#include "procgen/wfc.h"

namespace src::demo {

// 4. Wave Function Collapse : 4 tuiles (Mer, Plage, Herbe, Foret) et des
// regles de voisinage modifiables. Clic gauche = poser une tuile a la main.
class WfcDemo final : public Demo {
 public:
  WfcDemo();

  [[nodiscard]] const char* name() const override { return "4. Wave Function Collapse"; }
  [[nodiscard]] const char* file_tag() const override { return "wfc"; }

  void Gui() override;
  void Update(float dt) override;
  void Draw(sf::RenderTarget& target) override;
  void NextStep() override;
  void OnMouse(sf::Vector2f map_pos, bool left, bool right) override;

 protected:
  void Regenerate() override;

 private:
  static constexpr int kWidth = 48;  // cases de 20 pixels
  static constexpr int kHeight = 36;
  static constexpr int kTiles = 4;

  void ApplyRules();  // recopie les poids et les regles dans le Wfc
  void Refresh();

  core::procgen::Wfc wfc_;
  view::GridView view_;

  std::array<float, kTiles> weights_ = {3.0f, 1.0f, 2.0f, 2.0f};
  std::array<std::array<bool, kTiles>, kTiles> rules_{};
  bool running_ = true;
  int speed_ = 4;  // etapes par image
  bool guide_ = false;
  float guide_bias_ = 20.0f;
  float guide_zoom_ = 0.07f;
  int brush_ = 0;
  std::string message_;
};

}  // namespace src::demo

#endif  // SRC_DEMOS_WFC_DEMO_H
