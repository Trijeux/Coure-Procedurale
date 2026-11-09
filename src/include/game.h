#ifndef SRC_GAME_H
#define SRC_GAME_H

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>
#include <string>
#include <vector>

#include "demo.h"

namespace src::game {

class Game {
 public:
  Game();
  ~Game();

  Game(const Game&) = delete;
  Game& operator=(const Game&) = delete;

  void Run();

 private:
  void ProcessEvents();
  void Update(sf::Time dt);
  void Render();

  void Panel();
  void SelectDemo(int index);

  // Ou est la carte dans la fenetre (elle est mise a l'echelle).
  [[nodiscard]] sf::FloatRect MapRect() const;

  // Captures d'ecran dans le dossier "captures/".
  void SaveMap();
  void SaveWindow();
  std::string NextCaptureName(const std::string& suffix) const;

  sf::RenderWindow window_;
  sf::RenderTexture map_;
  std::vector<std::unique_ptr<demo::Demo>> demos_;
  int current_ = 0;

  bool capture_window_ = false;  // capture demandee pour la fin de l'image
  std::string last_capture_;
};

}  // namespace src::game

#endif  // SRC_GAME_H
