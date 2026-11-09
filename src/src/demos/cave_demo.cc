#include "demos/cave_demo.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <imgui.h>

#include <algorithm>

namespace src::demo {

using core::procgen::Cell;

CaveDemo::CaveDemo() {
  // Reglages de depart (voir la presentation pour l'effet de chacun).
  settings_.width = 120;
  settings_.height = 90;
  settings_.fill_ratio = 0.35f;
  settings_.walkers = 1;
  settings_.brush = 1;
  settings_.straight_bias = 0.3f;
  settings_.lifetime = 200;
  Regenerate();
}

void CaveDemo::Regenerate() {
  passes_ = 0;
  removed_ = 0;
  if (mode_ == kDrunkard) {
    walk_.Start(settings_, seed_);
    digging_ = true;
    if (!animate_) {
      walk_.Finish();
      digging_ = false;
    }
    grid_ = walk_.grid();
  } else {
    core::procgen::Random random(seed_);
    grid_ = core::procgen::Grid<Cell>(settings_.width, settings_.height);
    core::procgen::RandomFill(grid_, random, wall_ratio_);
    digging_ = false;
  }
  Refresh();
}

void CaveDemo::AutomatonPass() {
  if (digging_) {
    walk_.Finish();
    grid_ = walk_.grid();
    digging_ = false;
  }
  grid_ = core::procgen::CaveStep(grid_, threshold_);
  passes_++;
  Refresh();
}

void CaveDemo::Clean() {
  removed_ += core::procgen::KeepLargestRegion(grid_);
  Refresh();
}

void CaveDemo::DoEverything() {
  if (digging_) {
    walk_.Finish();
    grid_ = walk_.grid();
    digging_ = false;
  }
  for (int i = 0; i < auto_passes_; i++) {
    grid_ = core::procgen::CaveStep(grid_, threshold_);
    passes_++;
  }
  removed_ += core::procgen::KeepLargestRegion(grid_);
  Refresh();
}

void CaveDemo::Update(float /*dt*/) {
  if (!digging_) {
    return;
  }
  for (int i = 0; i < speed_; i++) {
    if (!walk_.Step()) {
      digging_ = false;
      break;
    }
  }
  grid_ = walk_.grid();
  Refresh();
}

void CaveDemo::Refresh() {
  regions_ = core::procgen::FindRegions(grid_);
  view_.Update(grid_.width(), grid_.height(), [this](int x, int y) {
    if (grid_.At(x, y) == Cell::kWall) {
      return view::palette::kWall;
    }
    if (color_regions_ && regions_.sizes.size() > 1) {
      return view::ColorFromIndex(regions_.labels.At(x, y));
    }
    return view::palette::kFloor;
  });
}

void CaveDemo::Gui() {
  ImGui::SeparatorText("Depart");
  if (ImGui::Combo("Mode", &mode_, "Drunkard's Walk\0Automate seul (au hasard)\0")) {
    Regenerate();
  }

  bool changed = false;
  if (mode_ == kDrunkard) {
    changed |= ImGui::SliderFloat("A creuser", &settings_.fill_ratio, 0.1f, 0.7f, "%.2f");
    changed |= ImGui::SliderInt("Ivrognes", &settings_.walkers, 1, 8);
    changed |= ImGui::SliderInt("Pinceau", &settings_.brush, 1, 4);
    ImGui::TextDisabled("1 = couloirs d'une case");
    changed |= ImGui::SliderFloat("Tout droit", &settings_.straight_bias, 0.0f, 0.9f, "%.2f");
    ImGui::TextDisabled("chance de garder sa direction");
    changed |= ImGui::SliderInt("Duree de vie", &settings_.lifetime, 0, 2000);
    ImGui::TextDisabled("0 = jamais, sinon il reapparait\nsur une case deja creusee");
    ImGui::Checkbox("Animation", &animate_);
    if (animate_) {
      ImGui::SliderInt("Vitesse", &speed_, 1, 2000, "%d pas/image", ImGuiSliderFlags_Logarithmic);
    }
    if (digging_) {
      ImGui::ProgressBar(static_cast<float>(walk_.dug()) / static_cast<float>(std::max(1, walk_.target())),
                         ImVec2(-1.0f, 0.0f), "on creuse...");
    }
  } else {
    changed |= ImGui::SliderFloat("Murs au depart", &wall_ratio_, 0.3f, 0.7f, "%.2f");
  }
  if (changed) {
    Regenerate();
  }

  SeedGui();

  ImGui::SeparatorText("Etapes");
  if (ImGui::Button("1. Recommencer", ImVec2(-1.0f, 0.0f))) {
    Regenerate();
  }
  if (ImGui::Button("2. Une passe d'automate (Espace)", ImVec2(-1.0f, 0.0f))) {
    AutomatonPass();
  }
  if (ImGui::Button("3. Nettoyer (flood fill)", ImVec2(-1.0f, 0.0f))) {
    Clean();
  }
  ImGui::SliderInt("Passes", &auto_passes_, 0, 10);
  if (ImGui::Button("Tout faire d'un coup", ImVec2(-1.0f, 0.0f))) {
    DoEverything();
  }

  ImGui::SeparatorText("Automate");
  ImGui::SliderInt("Seuil", &threshold_, 1, 7);
  ImGui::TextDisabled("> seuil murs autour : mur\n< seuil murs autour : sol");

  ImGui::SeparatorText("Resultat");
  int floor = 0;
  for (const int s : regions_.sizes) {
    floor += s;
  }
  const int inside = (grid_.width() - 2) * (grid_.height() - 2);
  ImGui::Text("Passes d'automate : %d", passes_);
  ImGui::Text("Sol : %d %%", inside > 0 ? floor * 100 / inside : 0);
  if (regions_.sizes.size() > 1) {
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "Zones separees : %d",
                       static_cast<int>(regions_.sizes.size()));
  } else {
    ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), "Tout est relie !");
  }
  if (removed_ > 0) {
    ImGui::Text("Zones rebouchees : %d", removed_);
  }
  if (ImGui::Checkbox("Colorer les zones", &color_regions_)) {
    Refresh();
  }
}

void CaveDemo::Draw(sf::RenderTarget& target) {
  target.clear(view::palette::kBackground);
  const sf::Vector2f size{static_cast<float>(kMapWidth), static_cast<float>(kMapHeight)};
  view_.Draw(target, size);

  // Les ivrognes en rouge pendant qu'ils creusent.
  if (digging_) {
    const float cw = size.x / static_cast<float>(grid_.width());
    const float ch = size.y / static_cast<float>(grid_.height());
    sf::RectangleShape walker({cw, ch});
    walker.setFillColor(view::palette::kWalker);
    for (const auto& p : walk_.walkers()) {
      walker.setPosition({static_cast<float>(p.x) * cw, static_cast<float>(p.y) * ch});
      target.draw(walker);
    }
  }
}

}  // namespace src::demo
