#include "demo.h"

#include <imgui.h>

#include <random>

namespace src::demo {

std::uint32_t RandomSeed() {
  static std::random_device device;
  return device() % 1000000u;
}

void Demo::SeedGui() {
  ImGui::SeparatorText("Seed");
  ImGui::SetNextItemWidth(130.0f);
  if (ImGui::InputScalar("##seed", ImGuiDataType_U32, &seed_)) {
    Regenerate();
  }
  ImGui::SameLine();
  if (ImGui::Button("Nouvelle seed (R)")) {
    SetSeed(RandomSeed());
  }
}

}  // namespace src::demo
