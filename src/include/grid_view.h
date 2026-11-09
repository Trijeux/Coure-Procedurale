#ifndef SRC_GRID_VIEW_H
#define SRC_GRID_VIEW_H

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <functional>

namespace src::view {

// Affiche une grille de couleurs : 1 case = 1 pixel d'une image,
// puis l'image est etiree sur toute la carte (sans flou).
class GridView {
 public:
  void Update(int width, int height, const std::function<sf::Color(int x, int y)>& color);
  void Draw(sf::RenderTarget& target, sf::Vector2f size) const;

 private:
  sf::Image image_;
  sf::Texture texture_;
};

// Couleurs communes a toutes les demos.
namespace palette {
inline const sf::Color kBackground{24, 22, 30};
inline const sf::Color kWall{40, 34, 46};
inline const sf::Color kFloor{214, 196, 160};
inline const sf::Color kWalker{230, 70, 60};
inline const sf::Color kCorridor{170, 150, 120};
inline const sf::Color kAccent{224, 166, 58};
}  // namespace palette

// Couleur differente pour chaque numero (pour colorer les zones).
sf::Color ColorFromIndex(int index);

}  // namespace src::view

#endif  // SRC_GRID_VIEW_H
