#include "demos/bsp_demo.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <imgui.h>

#include <algorithm>

#include "grid_view.h"

namespace src::demo {

using core::procgen::BspNode;
using core::procgen::Point;
using core::procgen::Rect;

namespace {

// 96 x 72 cases de 10 pixels = 960 x 720.
constexpr float kCell = 10.0f;

sf::Vector2f ToPixels(int x, int y) {
  return {static_cast<float>(x) * kCell, static_cast<float>(y) * kCell};
}

// Une couleur par profondeur : les premieres decoupes sont les plus visibles.
sf::Color DepthColor(int depth) {
  static const sf::Color kColors[] = {
      {224, 166, 58}, {230, 110, 80}, {200, 90, 160}, {110, 120, 220},
      {70, 170, 200}, {90, 190, 120}, {180, 180, 180}, {140, 140, 140}};
  return kColors[std::min(depth, 7)];
}

void FillCells(sf::RenderTarget& target, const Rect& r, sf::Color color) {
  sf::RectangleShape shape({static_cast<float>(r.w) * kCell, static_cast<float>(r.h) * kCell});
  shape.setPosition(ToPixels(r.x, r.y));
  shape.setFillColor(color);
  target.draw(shape);
}

}  // namespace

BspDemo::BspDemo() {
  settings_.room_fill = 0.5f;
  Regenerate();
}

void BspDemo::Regenerate() {
  settings_.width = 96;
  settings_.height = 72;
  bsp_.Generate(settings_, seed_);
  shown_depth_ = bsp_.depth_reached();  // on montre tout
}

void BspDemo::NextStep() {
  // Espace : une profondeur de plus (et on revient a 0 a la fin).
  shown_depth_ = shown_depth_ >= bsp_.depth_reached() ? 0 : shown_depth_ + 1;
}

void BspDemo::Update(float dt) {
  if (!animate_) {
    return;
  }
  timer_ += dt;
  if (timer_ > 0.8f) {
    timer_ = 0.0f;
    NextStep();
  }
}

void BspDemo::Gui() {
  bool changed = false;

  ImGui::SeparatorText("Decoupe");
  changed |= ImGui::SliderInt("Profondeur max", &settings_.max_depth, 1, 8);
  changed |= ImGui::SliderInt("Zone mini", &settings_.min_leaf, 6, 30);
  changed |= ImGui::SliderFloat("Coupe entre", &settings_.min_ratio, 0.2f, 0.5f, "%.2f");
  ImGui::TextDisabled("%.0f %% et %.0f %% de la zone", settings_.min_ratio * 100.0f,
                      (1.0f - settings_.min_ratio) * 100.0f);
  changed |= ImGui::Checkbox("Couper le cote le plus long", &settings_.split_longest);

  ImGui::SeparatorText("Salles");
  changed |= ImGui::SliderFloat("Taille mini", &settings_.room_fill, 0.2f, 1.0f, "%.2f");
  ImGui::TextDisabled("par rapport a sa zone");
  changed |= ImGui::SliderInt("Marge", &settings_.room_margin, 1, 4);

  SeedGui();
  if (changed) {
    Regenerate();
  }

  ImGui::SeparatorText("Affichage");
  ImGui::SliderInt("Profondeur montree", &shown_depth_, 0, bsp_.depth_reached());
  ImGui::Checkbox("Animer (Espace = +1)", &animate_);
  ImGui::Checkbox("Decoupes", &show_cuts_);
  ImGui::Checkbox("Salles", &show_rooms_);
  ImGui::Checkbox("Couloirs", &show_corridors_);

  ImGui::SeparatorText("Resultat");
  ImGui::Text("Salles (feuilles) : %d", bsp_.leaf_count());
  ImGui::Text("Couloirs : %d", static_cast<int>(bsp_.corridors().size()));
  ImGui::Text("Profondeur atteinte : %d", bsp_.depth_reached());
  ImGui::TextDisabled("Couloirs = salles - 1 : c'est un arbre !");
}

void BspDemo::DrawNode(sf::RenderTarget& target, const BspNode& node, bool rooms) const {
  const bool cut_visible = !node.IsLeaf() && node.depth < shown_depth_;
  if (!cut_visible) {
    // On s'arrete ici : on dessine la salle si c'est une vraie feuille.
    if (rooms && node.IsLeaf() && node.room) {
      FillCells(target, *node.room, view::palette::kFloor);
    }
    return;
  }
  DrawNode(target, *node.left, rooms);
  DrawNode(target, *node.right, rooms);

  if (show_cuts_) {
    // La ligne de coupe entre les deux enfants.
    const Rect& a = node.area;
    const float thickness = std::max(2.0f, 8.0f - 1.5f * static_cast<float>(node.depth));
    sf::RectangleShape line;
    if (node.vertical_cut) {
      const int x = node.left->area.x + node.left->area.w;
      line.setSize({thickness, static_cast<float>(a.h) * kCell});
      line.setPosition({static_cast<float>(x) * kCell - thickness / 2.0f, static_cast<float>(a.y) * kCell});
    } else {
      const int y = node.left->area.y + node.left->area.h;
      line.setSize({static_cast<float>(a.w) * kCell, thickness});
      line.setPosition({static_cast<float>(a.x) * kCell, static_cast<float>(y) * kCell - thickness / 2.0f});
    }
    line.setFillColor(DepthColor(node.depth));
    target.draw(line);
  }
}

void BspDemo::Draw(sf::RenderTarget& target) {
  target.clear(view::palette::kWall);

  const bool complete = shown_depth_ >= bsp_.depth_reached();

  // Les couloirs d'abord (sous les salles), seulement quand tout est decoupe.
  if (show_corridors_ && complete) {
    sf::RectangleShape cell({kCell, kCell});
    cell.setFillColor(view::palette::kCorridor);
    const auto draw_line = [&](Point from, Point to) {
      const int dx = (to.x > from.x) - (to.x < from.x);
      const int dy = (to.y > from.y) - (to.y < from.y);
      Point p = from;
      while (true) {
        cell.setPosition(ToPixels(p.x, p.y));
        target.draw(cell);
        if (p.x == to.x && p.y == to.y) {
          break;
        }
        p.x += dx;
        p.y += dy;
      }
    };
    for (const auto& c : bsp_.corridors()) {
      draw_line(c.from, c.corner);
      draw_line(c.corner, c.to);
    }
  }

  // Les salles n'apparaissent qu'une fois la decoupe terminee.
  DrawNode(target, bsp_.root(), show_rooms_ && complete);

  // Le cadre de la carte.
  sf::RectangleShape frame({static_cast<float>(bsp_.root().area.w) * kCell,
                            static_cast<float>(bsp_.root().area.h) * kCell});
  frame.setPosition(ToPixels(bsp_.root().area.x, bsp_.root().area.y));
  frame.setFillColor(sf::Color::Transparent);
  frame.setOutlineColor(DepthColor(0));
  frame.setOutlineThickness(2.0f);
  target.draw(frame);
}

}  // namespace src::demo
