#include "demos/wfc_demo.h"

#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <vector>

#include "procgen/noise.h"

namespace src::demo {

using core::procgen::Wfc;

namespace {

constexpr const char* kNames[] = {"Mer", "Plage", "Herbe", "Foret"};
const sf::Color kColors[] = {{59, 127, 196}, {241, 212, 138}, {123, 182, 97}, {47, 107, 58}};

ImVec4 ToImGui(sf::Color c) {
  return {static_cast<float>(c.r) / 255.0f, static_cast<float>(c.g) / 255.0f,
          static_cast<float>(c.b) / 255.0f, 1.0f};
}

}  // namespace

WfcDemo::WfcDemo() {
  // Regles du cours : chaque tuile touche elle-meme et ses voisines dans la
  // chaine Mer - Plage - Herbe - Foret. Jamais de foret les pieds dans l'eau.
  for (int i = 0; i < kTiles; i++) {
    rules_[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)] = true;
    if (i + 1 < kTiles) {
      rules_[static_cast<std::size_t>(i)][static_cast<std::size_t>(i + 1)] = true;
      rules_[static_cast<std::size_t>(i + 1)][static_cast<std::size_t>(i)] = true;
    }
  }
  Regenerate();
}

void WfcDemo::ApplyRules() {
  std::vector<Wfc::Tile> tiles;
  for (int i = 0; i < kTiles; i++) {
    tiles.push_back({kNames[i], weights_[static_cast<std::size_t>(i)]});
  }
  wfc_.SetTiles(tiles);
  for (int a = 0; a < kTiles; a++) {
    for (int b = a; b < kTiles; b++) {
      if (rules_[static_cast<std::size_t>(a)][static_cast<std::size_t>(b)]) {
        wfc_.Allow(a, b);
      }
    }
  }
}

void WfcDemo::Regenerate() {
  ApplyRules();
  message_.clear();

  std::vector<Wfc::Mask> guide;
  std::vector<int> preferred;
  if (guide_) {
    // Le guide : un Perlin donne une "hauteur" a chaque case. On autorise la
    // tuile de cette hauteur et ses voisines, et on prefere la bonne tuile.
    core::procgen::Noise noise(seed_);
    for (int y = 0; y < kHeight; y++) {
      for (int x = 0; x < kWidth; x++) {
        const float h = noise.Fbm(static_cast<float>(x) * guide_zoom_,
                                  static_cast<float>(y) * guide_zoom_, 3, 0.5f, 2.0f) * 0.5f + 0.5f;
        const int tile = std::clamp(static_cast<int>((h - 0.3f) / 0.4f * static_cast<float>(kTiles)), 0, kTiles - 1);
        Wfc::Mask mask = 0;
        for (int t = std::max(0, tile - 1); t <= std::min(kTiles - 1, tile + 1); t++) {
          mask |= Wfc::Mask{1} << static_cast<unsigned>(t);
        }
        guide.push_back(mask);
        preferred.push_back(tile);
      }
    }
  }
  wfc_.SetPreferred(preferred, guide_bias_);
  wfc_.Start(kWidth, kHeight, seed_, guide);
  if (wfc_.guide_failed()) {
    message_ = "Le guide est impossible avec ces regles : ignore.";
  }
  Refresh();
}

void WfcDemo::NextStep() {
  running_ = false;
  if (wfc_.Step() == Wfc::Status::kContradiction) {
    message_ = "Contradiction ! On recommence.";
  }
  Refresh();
}

void WfcDemo::Update(float /*dt*/) {
  if (!running_ || wfc_.done()) {
    return;
  }
  for (int i = 0; i < speed_; i++) {
    const Wfc::Status status = wfc_.Step();
    if (status == Wfc::Status::kContradiction) {
      message_ = "Contradiction ! On recommence.";
    }
    if (status == Wfc::Status::kDone) {
      break;
    }
  }
  Refresh();
}

void WfcDemo::OnMouse(sf::Vector2f map_pos, bool left, bool /*right*/) {
  if (!left) {
    return;
  }
  const float cell_w = static_cast<float>(kMapWidth) / static_cast<float>(kWidth);
  const float cell_h = static_cast<float>(kMapHeight) / static_cast<float>(kHeight);
  const int x = static_cast<int>(map_pos.x / cell_w);
  const int y = static_cast<int>(map_pos.y / cell_h);
  if (x < 0 || y < 0 || x >= kWidth || y >= kHeight) {
    return;
  }
  if (wfc_.CollapsedTile(x, y) == brush_) {
    return;
  }
  if (wfc_.Force(x, y, brush_)) {
    message_.clear();
  } else {
    message_ = std::string(kNames[brush_]) + " impossible ici (regles des voisines)";
  }
  Refresh();
}

void WfcDemo::Refresh() {
  view_.Update(kWidth, kHeight, [this](int x, int y) {
    const Wfc::Mask options = wfc_.Options(x, y);
    const int count = wfc_.OptionCount(x, y);
    if (count == 0) {
      return sf::Color(255, 0, 255);  // contradiction (ne reste jamais affichee)
    }
    // Moyenne des couleurs encore possibles : on "voit" la superposition.
    int r = 0;
    int g = 0;
    int b = 0;
    for (int t = 0; t < kTiles; t++) {
      if ((options & (Wfc::Mask{1} << static_cast<unsigned>(t))) != 0) {
        r += kColors[t].r;
        g += kColors[t].g;
        b += kColors[t].b;
      }
    }
    r /= count;
    g /= count;
    b /= count;
    if (count > 1) {
      // Plus il reste de choix, plus la case est sombre.
      const int dark = 40 + 30 * (count - 1);
      r = r * (255 - dark) / 255;
      g = g * (255 - dark) / 255;
      b = b * (255 - dark) / 255;
    }
    return sf::Color(static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g),
                     static_cast<std::uint8_t>(b));
  });
}

void WfcDemo::Gui() {
  ImGui::SeparatorText("Lecture");
  ImGui::Checkbox("En marche", &running_);
  ImGui::SameLine();
  if (ImGui::Button("1 etape (Espace)")) {
    NextStep();
  }
  ImGui::SliderInt("Vitesse", &speed_, 1, 200, "%d etapes/image", ImGuiSliderFlags_Logarithmic);
  if (ImGui::Button("Tout finir")) {
    while (wfc_.Step() != Wfc::Status::kDone) {
    }
    Refresh();
  }
  ImGui::SameLine();
  if (ImGui::Button("Recommencer")) {
    Regenerate();
  }
  ImGui::ProgressBar(static_cast<float>(wfc_.collapsed_count()) / static_cast<float>(kWidth * kHeight),
                     ImVec2(-1.0f, 0.0f));
  ImGui::Text("Contradictions : %d", wfc_.contradictions());

  ImGui::SeparatorText("Tuiles et poids");
  bool changed = false;
  for (int i = 0; i < kTiles; i++) {
    ImGui::PushID(i);  // chaque ligne a ses propres identifiants ImGui
    ImGui::ColorButton("##couleur", ToImGui(kColors[i]), ImGuiColorEditFlags_NoTooltip, ImVec2(18, 18));
    ImGui::SameLine();
    changed |= ImGui::SliderFloat(kNames[i], &weights_[static_cast<std::size_t>(i)], 0.05f, 10.0f,
                                  "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::PopID();
  }

  ImGui::SeparatorText("Qui peut toucher qui ?");
  if (ImGui::BeginTable("regles", kTiles + 1, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    for (int i = 0; i < kTiles; i++) {
      ImGui::TableNextColumn();
      ImGui::TextColored(ToImGui(kColors[i]), "%.4s", kNames[i]);
    }
    for (int a = 0; a < kTiles; a++) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextColored(ToImGui(kColors[a]), "%s", kNames[a]);
      for (int b = 0; b < kTiles; b++) {
        ImGui::TableNextColumn();
        ImGui::PushID(a * kTiles + b);
        bool& rule = rules_[static_cast<std::size_t>(a)][static_cast<std::size_t>(b)];
        if (ImGui::Checkbox("##r", &rule)) {
          // La regle marche dans les deux sens.
          rules_[static_cast<std::size_t>(b)][static_cast<std::size_t>(a)] = rule;
          changed = true;
        }
        ImGui::PopID();
      }
    }
    ImGui::EndTable();
  }

  ImGui::SeparatorText("Guide");
  changed |= ImGui::Checkbox("Guider avec un Perlin", &guide_);
  if (guide_) {
    changed |= ImGui::SliderFloat("Force", &guide_bias_, 1.0f, 100.0f, "x %.0f", ImGuiSliderFlags_Logarithmic);
    changed |= ImGui::SliderFloat("Zoom##guide", &guide_zoom_, 0.02f, 0.2f, "%.3f");
  }
  ImGui::TextDisabled("Sans guide : le WFC ne connait que\nles voisines, pas la forme globale.");

  SeedGui();
  if (changed) {
    Regenerate();
  }

  ImGui::SeparatorText("Pinceau (clic gauche)");
  for (int i = 0; i < kTiles; i++) {
    if (i > 0) {
      ImGui::SameLine();
    }
    ImGui::PushID(100 + i);
    ImGui::RadioButton(kNames[i], &brush_, i);
    ImGui::PopID();
  }
  if (!message_.empty()) {
    ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.4f, 1.0f), "%s", message_.c_str());
  }
}

void WfcDemo::Draw(sf::RenderTarget& target) {
  target.clear(view::palette::kBackground);
  view_.Draw(target, {static_cast<float>(kMapWidth), static_cast<float>(kMapHeight)});
}

}  // namespace src::demo
