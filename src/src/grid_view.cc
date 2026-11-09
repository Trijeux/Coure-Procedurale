#include "grid_view.h"

#include <SFML/Graphics/Sprite.hpp>
#include <cmath>
#include <cstdint>

namespace src::view {

void GridView::Update(int width, int height,
                      const std::function<sf::Color(int x, int y)>& color) {
  const sf::Vector2u size{static_cast<unsigned>(width), static_cast<unsigned>(height)};
  if (image_.getSize() != size) {
    image_.resize(size);
  }
  for (unsigned y = 0; y < size.y; y++) {
    for (unsigned x = 0; x < size.x; x++) {
      image_.setPixel({x, y}, color(static_cast<int>(x), static_cast<int>(y)));
    }
  }
  if (texture_.getSize() != size) {
    if (!texture_.resize(size)) {
      return;
    }
  }
  texture_.update(image_);
}

void GridView::Draw(sf::RenderTarget& target, sf::Vector2f size) const {
  if (texture_.getSize().x == 0) {
    return;
  }
  sf::Sprite sprite(texture_);
  sprite.setScale({size.x / static_cast<float>(texture_.getSize().x),
                   size.y / static_cast<float>(texture_.getSize().y)});
  target.draw(sprite);
}

sf::Color ColorFromIndex(int index) {
  // On tourne de 137.5 degres (l'angle d'or) a chaque numero :
  // deux zones voisines ont des teintes bien differentes.
  const float hue = std::fmod(static_cast<float>(index) * 137.508f, 360.0f) / 60.0f;
  const float x = 1.0f - std::fabs(std::fmod(hue, 2.0f) - 1.0f);
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  switch (static_cast<int>(hue)) {
    case 0: r = 1.0f; g = x; break;
    case 1: r = x; g = 1.0f; break;
    case 2: g = 1.0f; b = x; break;
    case 3: g = x; b = 1.0f; break;
    case 4: r = x; b = 1.0f; break;
    default: r = 1.0f; b = x; break;
  }
  const auto channel = [](float v) { return static_cast<std::uint8_t>(90.0f + v * 150.0f); };
  return {channel(r), channel(g), channel(b)};
}

}  // namespace src::view
