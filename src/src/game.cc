#include "game.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string_view>

#include "demos/bsp_demo.h"
#include "demos/cave_demo.h"
#include "demos/markov_demo.h"
#include "demos/perlin_demo.h"
#include "demos/wfc_demo.h"
#include "grid_view.h"

namespace src::game {

namespace {

constexpr float kPanelWidth = 320.0f;
constexpr const char* kCaptureFolder = "captures";

sf::String Utf8(std::string_view text) { return sf::String::fromUtf8(text.begin(), text.end()); }

}  // namespace

Game::Game()
    : window_(sf::VideoMode({static_cast<unsigned>(kPanelWidth) + demo::kMapWidth, demo::kMapHeight}),
              Utf8("Génération procédurale")) {
  window_.setFramerateLimit(60);
  if (!map_.resize({demo::kMapWidth, demo::kMapHeight})) {
    throw std::runtime_error("Impossible de creer la RenderTexture de la carte");
  }
  if (!ImGui::SFML::Init(window_)) {
    throw std::runtime_error("Impossible d'initialiser ImGui");
  }
  ImGui::GetIO().IniFilename = nullptr;  // pas de fichier imgui.ini

  demos_.push_back(std::make_unique<demo::MarkovDemo>());
  demos_.push_back(std::make_unique<demo::PerlinDemo>());
  demos_.push_back(std::make_unique<demo::CaveDemo>());
  demos_.push_back(std::make_unique<demo::BspDemo>());
  demos_.push_back(std::make_unique<demo::WfcDemo>());
}

Game::~Game() { ImGui::SFML::Shutdown(); }

void Game::Run() {
  sf::Clock clock;
  while (window_.isOpen()) {
    const sf::Time dt = clock.restart();
    ProcessEvents();
    Update(dt);
    Render();
  }
}

void Game::SelectDemo(int index) {
  if (index >= 0 && index < static_cast<int>(demos_.size())) {
    current_ = index;
  }
}

sf::FloatRect Game::MapRect() const {
  // La carte garde ses proportions et remplit la place a droite du panneau.
  const auto size = sf::Vector2f(window_.getSize());
  const float available_w = std::max(1.0f, size.x - kPanelWidth);
  const float scale = std::min(available_w / static_cast<float>(demo::kMapWidth),
                               size.y / static_cast<float>(demo::kMapHeight));
  const sf::Vector2f map_size{static_cast<float>(demo::kMapWidth) * scale,
                              static_cast<float>(demo::kMapHeight) * scale};
  const sf::Vector2f origin{kPanelWidth + (available_w - map_size.x) / 2.0f,
                            (size.y - map_size.y) / 2.0f};
  return {origin, map_size};
}

void Game::ProcessEvents() {
  while (const std::optional event = window_.pollEvent()) {
    ImGui::SFML::ProcessEvent(window_, *event);

    if (event->is<sf::Event::Closed>()) {
      window_.close();
    } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
      window_.setView(sf::View(sf::FloatRect({0.0f, 0.0f}, sf::Vector2f(resized->size))));
    } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
      if (ImGui::GetIO().WantCaptureKeyboard) {
        continue;  // on est en train d'ecrire dans un champ
      }
      using sf::Keyboard::Key;
      switch (key->code) {
        case Key::Num1: SelectDemo(0); break;
        case Key::Num2: SelectDemo(1); break;
        case Key::Num3: SelectDemo(2); break;
        case Key::Num4: SelectDemo(3); break;
        case Key::Num5: SelectDemo(4); break;
        case Key::R: demos_[static_cast<std::size_t>(current_)]->SetSeed(demo::RandomSeed()); break;
        case Key::Space: demos_[static_cast<std::size_t>(current_)]->NextStep(); break;
        case Key::F11: capture_window_ = true; break;
        case Key::F12: SaveMap(); break;
        case Key::Escape: window_.close(); break;
        default: break;
      }
    }
  }
}

void Game::Update(sf::Time dt) {
  ImGui::SFML::Update(window_, dt);
  demo::Demo& demo = *demos_[static_cast<std::size_t>(current_)];

  // La souris sur la carte (si ImGui ne l'utilise pas).
  if (!ImGui::GetIO().WantCaptureMouse) {
    const sf::FloatRect rect = MapRect();
    const sf::Vector2f mouse(sf::Mouse::getPosition(window_));
    if (rect.contains(mouse)) {
      const sf::Vector2f in_map{(mouse.x - rect.position.x) / rect.size.x * static_cast<float>(demo::kMapWidth),
                                (mouse.y - rect.position.y) / rect.size.y * static_cast<float>(demo::kMapHeight)};
      demo.OnMouse(in_map, sf::Mouse::isButtonPressed(sf::Mouse::Button::Left),
                   sf::Mouse::isButtonPressed(sf::Mouse::Button::Right));
    }
  }

  demo.Update(dt.asSeconds());
  Panel();
  const sf::FloatRect rect = MapRect();
  demo.OverlayGui(rect.position, rect.size);
}

void Game::Panel() {
  ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(ImVec2(kPanelWidth, static_cast<float>(window_.getSize().y)));
  ImGui::Begin("Generation procedurale", nullptr,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

  if (ImGui::BeginCombo("Demo", demos_[static_cast<std::size_t>(current_)]->name())) {
    for (int i = 0; i < static_cast<int>(demos_.size()); i++) {
      if (ImGui::Selectable(demos_[static_cast<std::size_t>(i)]->name(), i == current_)) {
        SelectDemo(i);
      }
    }
    ImGui::EndCombo();
  }
  ImGui::TextDisabled("Touches 1 a 5 pour changer");

  ImGui::PushItemWidth(150.0f);
  demos_[static_cast<std::size_t>(current_)]->Gui();
  ImGui::PopItemWidth();

  ImGui::SeparatorText("Captures");
  if (ImGui::Button("Carte (F12)")) {
    SaveMap();
  }
  ImGui::SameLine();
  if (ImGui::Button("Fenetre (F11)")) {
    capture_window_ = true;
  }
  if (!last_capture_.empty()) {
    ImGui::TextWrapped("Enregistre : %s", last_capture_.c_str());
  }
  ImGui::End();
}

void Game::Render() {
  demo::Demo& demo = *demos_[static_cast<std::size_t>(current_)];

  // 1. La demo dessine dans sa texture (toujours 960 x 720).
  map_.clear(view::palette::kBackground);
  demo.Draw(map_);
  map_.display();

  // 2. On affiche la texture a la bonne taille, puis ImGui par-dessus.
  window_.clear(view::palette::kBackground);
  const sf::FloatRect rect = MapRect();
  sf::Sprite sprite(map_.getTexture());
  sprite.setPosition(rect.position);
  sprite.setScale({rect.size.x / static_cast<float>(demo::kMapWidth),
                   rect.size.y / static_cast<float>(demo::kMapHeight)});
  window_.draw(sprite);
  ImGui::SFML::Render(window_);

  // 3. Capture de la fenetre : juste avant display(), l'image est complete.
  if (capture_window_) {
    capture_window_ = false;
    SaveWindow();
  }
  window_.display();
}

std::string Game::NextCaptureName(const std::string& suffix) const {
  std::error_code error;
  std::filesystem::create_directories(kCaptureFolder, error);
  const demo::Demo& demo = *demos_[static_cast<std::size_t>(current_)];
  const std::string base = std::string(kCaptureFolder) + "/" + demo.file_tag() + "_seed" +
                           std::to_string(demo.seed()) + suffix;
  std::string name = base + ".png";
  for (int i = 2; std::filesystem::exists(name, error); i++) {
    name = base + "_" + std::to_string(i) + ".png";
  }
  return name;
}

void Game::SaveMap() {
  if (!demos_[static_cast<std::size_t>(current_)]->draws_map()) {
    capture_window_ = true;  // Markov : la "carte" est vide, on prend la fenetre
    return;
  }
  const std::string name = NextCaptureName("");
  last_capture_ = map_.getTexture().copyToImage().saveToFile(name) ? name : "Erreur : " + name;
}

void Game::SaveWindow() {
  sf::Texture texture;
  if (!texture.resize(window_.getSize())) {
    last_capture_ = "Erreur de capture";
    return;
  }
  texture.update(window_);
  const std::string name = NextCaptureName("_fenetre");
  last_capture_ = texture.copyToImage().saveToFile(name) ? name : "Erreur : " + name;
}

}  // namespace src::game
