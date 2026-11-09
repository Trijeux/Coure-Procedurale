#ifndef SRC_DEMOS_BSP_DEMO_H
#define SRC_DEMOS_BSP_DEMO_H

#include "demo.h"
#include "procgen/bsp.h"

namespace src::demo {

// 3. BSP : decouper la carte en arbre, une salle par feuille, des couloirs.
class BspDemo final : public Demo {
 public:
  BspDemo();

  [[nodiscard]] const char* name() const override { return "3. BSP"; }
  [[nodiscard]] const char* file_tag() const override { return "bsp"; }

  void Gui() override;
  void Update(float dt) override;
  void Draw(sf::RenderTarget& target) override;
  void NextStep() override;

 protected:
  void Regenerate() override;

 private:
  void DrawNode(sf::RenderTarget& target, const core::procgen::BspNode& node, bool rooms) const;

  core::procgen::BspSettings settings_;
  core::procgen::Bsp bsp_;

  int shown_depth_ = 99;  // on montre les decoupes jusqu'a cette profondeur
  bool show_cuts_ = true;
  bool show_rooms_ = true;
  bool show_corridors_ = true;
  bool animate_ = false;
  float timer_ = 0.0f;
};

}  // namespace src::demo

#endif  // SRC_DEMOS_BSP_DEMO_H
