#include "demos/markov_demo.h"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>

#include "grid_view.h"
#include "procgen/random.h"

namespace src::demo {

namespace {

constexpr const char* kFolder = "resources/markov";

// Minuscules, et on ne garde que les lettres a-z (les accents sont des
// caracteres sur plusieurs octets en UTF-8 : la chaine les couperait en deux).
std::string Clean(const std::string& line) {
  std::string word;
  for (const char c : line) {
    const auto u = static_cast<unsigned char>(c);
    if (std::isalpha(u) != 0 && u < 128) {
      word += static_cast<char>(std::tolower(u));
    }
  }
  return word;
}

}  // namespace

MarkovDemo::MarkovDemo() {
  LoadCorpora();
  Train();
  Regenerate();
}

void MarkovDemo::LoadCorpora() {
  corpora_.clear();
  std::error_code error;
  if (!std::filesystem::is_directory(kFolder, error)) {
    return;
  }
  for (const auto& entry : std::filesystem::directory_iterator(kFolder, error)) {
    if (entry.path().extension() != ".txt") {
      continue;
    }
    Corpus corpus;
    corpus.name = entry.path().stem().string();
    std::ifstream file(entry.path());
    std::string line;
    while (std::getline(file, line)) {
      std::string word = Clean(line);
      if (!word.empty()) {
        corpus.words.push_back(word);
      }
    }
    if (!corpus.words.empty()) {
      corpora_.push_back(std::move(corpus));
    }
  }
  std::sort(corpora_.begin(), corpora_.end(),
            [](const Corpus& a, const Corpus& b) { return a.name > b.name; });  // "villes" en premier
  corpus_ = 0;
}

void MarkovDemo::Train() {
  chain_ = core::procgen::MarkovChain(order_);
  known_.clear();
  if (corpora_.empty()) {
    return;
  }
  const Corpus& corpus = corpora_[static_cast<std::size_t>(corpus_)];
  chain_.Train(corpus.words);
  known_.insert(corpus.words.begin(), corpus.words.end());
}

void MarkovDemo::Regenerate() {
  results_.clear();
  copies_ = 0;
  if (corpora_.empty()) {
    return;
  }
  core::procgen::Random random(seed_);
  int attempts = 0;
  while (static_cast<int>(results_.size()) < count_ && attempts < 10000) {
    attempts++;
    std::string word = chain_.Generate(random, max_length_ + 1);
    if (static_cast<int>(word.size()) < min_length_ ||
        static_cast<int>(word.size()) > max_length_) {
      continue;  // trop court ou trop long : on retire
    }
    const bool copy = known_.contains(word);
    copies_ += copy ? 1 : 0;
    results_.push_back({word, copy});
  }
}

void MarkovDemo::Gui() {
  if (corpora_.empty()) {
    ImGui::TextWrapped("Aucune liste trouvee dans %s/", kFolder);
    ImGui::TextWrapped("Lancez le programme depuis le dossier de build (les ressources y sont copiees).");
    if (ImGui::Button("Recharger")) {
      LoadCorpora();
      Train();
      Regenerate();
    }
    return;
  }

  ImGui::SeparatorText("Donnees");
  const std::string& current = corpora_[static_cast<std::size_t>(corpus_)].name;
  if (ImGui::BeginCombo("Liste", current.c_str())) {
    for (int i = 0; i < static_cast<int>(corpora_.size()); i++) {
      const Corpus& c = corpora_[static_cast<std::size_t>(i)];
      if (ImGui::Selectable(c.name.c_str(), i == corpus_)) {
        corpus_ = i;
        Train();
        Regenerate();
      }
    }
    ImGui::EndCombo();
  }
  ImGui::Text("%d mots dans la liste", static_cast<int>(corpora_[static_cast<std::size_t>(corpus_)].words.size()));
  if (ImGui::Button("Recharger les fichiers")) {
    LoadCorpora();
    Train();
    Regenerate();
  }

  ImGui::SeparatorText("Chaine");
  if (ImGui::SliderInt("Ordre", &order_, 1, 5)) {
    Train();
    Regenerate();
  }
  ImGui::TextDisabled("= nombre de lettres regardees");
  ImGui::Text("%d contextes appris", static_cast<int>(chain_.context_count()));

  ImGui::SeparatorText("Generation");
  bool changed = ImGui::SliderInt("Nombre", &count_, 5, 60);
  changed |= ImGui::SliderInt("Longueur min", &min_length_, 1, 10);
  changed |= ImGui::SliderInt("Longueur max", &max_length_, min_length_, 20);
  if (changed) {
    Regenerate();
  }
  SeedGui();

  ImGui::SeparatorText("Resultat");
  const int total = static_cast<int>(results_.size());
  const float ratio = total > 0 ? static_cast<float>(copies_) / static_cast<float>(total) : 0.0f;
  ImGui::Text("Copies de la liste : %d / %d", copies_, total);
  ImGui::ProgressBar(ratio, ImVec2(-1.0f, 0.0f));
  ImGui::TextDisabled("Ordre trop haut = il recopie la liste");
}

void MarkovDemo::OverlayGui(sf::Vector2f map_origin, sf::Vector2f map_size) {
  ImGui::SetNextWindowPos(ImVec2(map_origin.x, map_origin.y));
  ImGui::SetNextWindowSize(ImVec2(map_size.x, map_size.y));
  ImGui::Begin("Noms inventes", nullptr,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

  ImGui::SetWindowFontScale(1.8f);
  if (ImGui::BeginTable("noms", 3)) {
    for (const Result& r : results_) {
      ImGui::TableNextColumn();
      if (r.copy) {
        ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.6f, 1.0f), "%s (copie)", r.word.c_str());
      } else {
        // Une majuscule au debut, c'est plus joli.
        std::string pretty = r.word;
        pretty[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(pretty[0])));
        ImGui::TextColored(ImVec4(0.95f, 0.8f, 0.45f, 1.0f), "%s", pretty.c_str());
      }
    }
    ImGui::EndTable();
  }

  // ---- La table de probabilites pour un contexte
  ImGui::SetWindowFontScale(1.3f);
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Text("Qu'est-ce qui suit... ");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120.0f);
  ImGui::InputText("##contexte", context_, sizeof(context_));
  ImGui::SameLine();
  ImGui::TextDisabled("(^ = debut du mot)");

  // Le contexte doit faire exactement "ordre" lettres : on complete avec des ^.
  std::string context = Clean(context_);
  const auto order = static_cast<std::size_t>(chain_.order());
  if (context.size() > order) {
    context = context.substr(context.size() - order);
  }
  context = std::string(order - context.size(), core::procgen::MarkovChain::kStart) + context;

  auto successors = chain_.Successors(context);
  std::sort(successors.begin(), successors.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
  int total = 0;
  for (const auto& s : successors) {
    total += s.second;
  }
  ImGui::Text("Contexte \"%s\" : vu %d fois", context.c_str(), total);
  int shown = 0;
  for (const auto& [letter, count] : successors) {
    if (shown++ >= 8) {
      break;
    }
    const float p = static_cast<float>(count) / static_cast<float>(total);
    char label[64];
    std::snprintf(label, sizeof(label), "%d fois - %.0f %%", count, p * 100.0f);
    ImGui::Text(" %c ", letter == core::procgen::MarkovChain::kEnd ? '$' : letter);
    ImGui::SameLine();
    ImGui::ProgressBar(p, ImVec2(420.0f, 0.0f), label);
  }
  if (successors.empty()) {
    ImGui::TextDisabled("Jamais vu dans la liste.");
  }
  ImGui::End();
}

void MarkovDemo::Draw(sf::RenderTarget& target) { target.clear(view::palette::kBackground); }

}  // namespace src::demo
