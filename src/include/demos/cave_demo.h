#ifndef SRC_DEMOS_CAVE_DEMO_H
#define SRC_DEMOS_CAVE_DEMO_H

#include "demo.h"
#include "grid_view.h"
#include "procgen/caves.h"

namespace src::demo {

// 2. Grottes : Drunkard's Walk, puis automate cellulaire, puis flood fill.
// Chaque etape se lance a la main pour voir ce qu'elle change.
class CaveDemo final : public Demo {
 public:
  CaveDemo();

  [[nodiscard]] const char* name() const override { return "2. Drunkard + automate"; }
  [[nodiscard]] const char* file_tag() const override { return "grotte"; }

  void Gui() override;
  void Update(float dt) override;
  void Draw(sf::RenderTarget& target) override;
  void NextStep() override { AutomatonPass(); }

 protected:
  void Regenerate() override;

 private:
  enum Mode { kDrunkard = 0, kAutomatonOnly = 1 };

  void AutomatonPass();
  void Clean();
  void DoEverything();
  void Refresh();  // recalcule les zones et l'image

  core::procgen::DrunkardSettings settings_;
  core::procgen::DrunkardWalk walk_;
  core::procgen::Grid<core::procgen::Cell> grid_;
  core::procgen::Regions regions_;
  view::GridView view_;

  int mode_ = kDrunkard;
  bool digging_ = false;
  bool animate_ = true;
  int speed_ = 60;          // pas par image
  float wall_ratio_ = 0.50f;  // mode "automate seul"
  int threshold_ = 4;
  int passes_ = 0;
  int auto_passes_ = 4;
  int removed_ = 0;
  bool color_regions_ = false;
};

}  // namespace src::demo

#endif  // SRC_DEMOS_CAVE_DEMO_H
