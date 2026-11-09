#ifndef API_GRAPHICS_RESOURCE_SPRIT_H
#define API_GRAPHICS_RESOURCE_SPRIT_H

#include <SFML/Graphics/Texture.hpp>
#include <array>

namespace api::graphics {
class ResourceSprit {
 public:
  enum class Texture {
    kMax
  };

  void Setup();

  sf::Texture& texture(Texture resource_id);

 protected:
 private:
  const std::array<std::string, static_cast<int>(Texture::kMax)>
      names_textures_{};

  std::array<sf::Texture, static_cast<int>(Texture::kMax)> textures_;

  std::string path_ = "resources/sprite/";

  sf::Texture blank_texture_;

  void LoadAllTextures();
};
}  // namespace api::graphics

#endif  // API_GRAPHICS_RESOURCE_SPRIT_H
