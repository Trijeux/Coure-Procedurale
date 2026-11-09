#ifndef API_GRAPHICS_RESOURCE_FONT_H
#define API_GRAPHICS_RESOURCE_FONT_H

#include <SFML/Graphics/Font.hpp>
#include <array>

namespace api::graphics {
class ResourceFont {
 public:
  enum class Font { kMax };

  void Setup();

  sf::Font& font(Font resource_id);

 protected:
 private:
  const std::array<std::string, static_cast<int>(Font::kMax)> names_font_{};

  std::string path_ = "resources/font/";

  std::array<sf::Font, static_cast<int>(Font::kMax)> fonts_;

  sf::Font blank_font_;

  void LoadAllFonts();
};
}  // namespace api::graphics

#endif  // API_GRAPHICS_RESOURCE_FONT_H
