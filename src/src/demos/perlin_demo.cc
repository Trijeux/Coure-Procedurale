#include "demos/perlin_demo.h"

#include <imgui.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>

namespace src::demo {

namespace {

// Les seuils du cours (hauteur entre 0 et 1).
sf::Color TerrainColor(float h) {
  if (h < 0.35f) return {30, 70, 140};     // eau profonde
  if (h < 0.45f) return {50, 110, 190};    // eau
  if (h < 0.50f) return {225, 205, 140};   // sable
  if (h < 0.70f) return {95, 160, 80};     // herbe
  if (h < 0.85f) return {120, 110, 100};   // roche
  return {240, 240, 245};                  // neige
}

sf::Color Gray(float h) {
  const auto g = static_cast<std::uint8_t>(std::clamp(h, 0.0f, 1.0f) * 255.0f);
  return {g, g, g};
}

}  // namespace

PerlinDemo::PerlinDemo() { Regenerate(); }

void PerlinDemo::Regenerate() {
  noise_.Reseed(seed_);
  heights_.assign(static_cast<std::size_t>(kWidth) * kHeight, 0.0f);

  // 1. Une valeur entre -1 et 1 pour chaque case.
  for (int y = 0; y < kHeight; y++) {
    for (int x = 0; x < kWidth; x++) {
      const float px = (static_cast<float>(x) + offset_[0]) * zoom_;
      const float py = (static_cast<float>(y) + offset_[1]) * zoom_;
      float v = 0.0f;
      if (type_ == 0) {
        v = noise_.White(x + static_cast<int>(offset_[0]), y + static_cast<int>(offset_[1]));
      } else if (type_ == 1) {
        v = noise_.ValueFbm(px, py, octaves_, persistence_, lacunarity_);
      } else {
        v = noise_.Fbm(px, py, octaves_, persistence_, lacunarity_);
      }
      heights_[static_cast<std::size_t>(y * kWidth + x)] = v;
    }
  }

  // 2. Histogramme des valeurs brutes : ou tombent-elles vraiment ?
  histogram_.fill(0.0f);
  for (const float v : heights_) {
    const int bin = std::clamp(static_cast<int>((v + 1.0f) * 0.5f * 40.0f), 0, 39);
    histogram_[static_cast<std::size_t>(bin)] += 1.0f;
  }

  // 3. On ramene entre 0 et 1. "Etirer" = le plus bas devient 0, le plus haut 1.
  const auto [min_it, max_it] = std::minmax_element(heights_.begin(), heights_.end());
  const float min = *min_it;
  const float range = std::max(*max_it - min, 0.0001f);
  for (int y = 0; y < kHeight; y++) {
    for (int x = 0; x < kWidth; x++) {
      float& h = heights_[static_cast<std::size_t>(y * kWidth + x)];
      h = normalize_ ? (h - min) / range : h * 0.5f + 0.5f;

      // 4. Ile : plus on s'eloigne du centre, plus on descend.
      if (island_) {
        const float half_w = static_cast<float>(kWidth) / 2.0f;
        const float half_h = static_cast<float>(kHeight) / 2.0f;
        const float dx = (static_cast<float>(x) - half_w) / half_w;
        const float dy = (static_cast<float>(y) - half_h) / half_h;
        h += island_strength_ * (0.25f - (dx * dx + dy * dy));
      }
      h -= sea_level_;
    }
  }

  view_.Update(kWidth, kHeight, [this](int x, int y) {
    const float h = heights_[static_cast<std::size_t>(y * kWidth + x)];
    return display_ == 0 ? Gray(h) : TerrainColor(h);
  });
}

void PerlinDemo::Gui() {
  bool changed = false;

  ImGui::SeparatorText("Bruit");
  changed |= ImGui::Combo("Type", &type_, "Blanc (rand)\0Value Noise\0Perlin\0");
  if (type_ != 0) {
    changed |= ImGui::SliderFloat("Zoom", &zoom_, 0.002f, 0.1f, "%.4f", ImGuiSliderFlags_Logarithmic);
    ImGui::TextDisabled("Bruit(x * zoom, y * zoom)");
  }

  if (type_ != 0) {
    ImGui::SeparatorText("Octaves (fBm)");
    changed |= ImGui::SliderInt("Octaves", &octaves_, 1, 8);
    changed |= ImGui::SliderFloat("Persistance", &persistence_, 0.1f, 0.9f, "%.2f");
    ImGui::TextDisabled("amplitude x %.2f a chaque octave", persistence_);
    changed |= ImGui::SliderFloat("Lacunarite", &lacunarity_, 1.5f, 3.5f, "%.2f");
    ImGui::TextDisabled("frequence x %.2f a chaque octave", lacunarity_);
  }

  ImGui::SeparatorText("Deplacement");
  changed |= ImGui::DragFloat2("Decalage", offset_, 2.0f);
  ImGui::TextDisabled("Le monde est infini : on se promene dedans");

  ImGui::SeparatorText("Carte");
  changed |= ImGui::Combo("Affichage", &display_, "Gris (hauteur)\0Terrain (seuils)\0");
  changed |= ImGui::Checkbox("Etirer entre 0 et 1", &normalize_);
  changed |= ImGui::SliderFloat("Niveau de la mer", &sea_level_, -0.3f, 0.3f, "%.2f");
  changed |= ImGui::Checkbox("Ile", &island_);
  if (island_) {
    changed |= ImGui::SliderFloat("Force de l'ile", &island_strength_, 0.1f, 1.5f, "%.2f");
  }

  SeedGui();

  ImGui::SeparatorText("Valeurs brutes (-1 a 1)");
  ImGui::PlotHistogram("##histo", histogram_.data(), static_cast<int>(histogram_.size()), 0,
                       nullptr, 0.0f, FLT_MAX, ImVec2(-1.0f, 90.0f));
  ImGui::TextDisabled("Plus d'octaves = valeurs serrees au milieu");

  if (changed) {
    Regenerate();
  }
}

void PerlinDemo::Draw(sf::RenderTarget& target) {
  target.clear(view::palette::kBackground);
  view_.Draw(target, {static_cast<float>(kMapWidth), static_cast<float>(kMapHeight)});
}

}  // namespace src::demo
