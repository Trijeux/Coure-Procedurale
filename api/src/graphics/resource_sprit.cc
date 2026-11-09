#include "graphics/resource_sprit.h"

#include <iostream>

void api::graphics::ResourceSprit::Setup() { LoadAllTextures(); }

sf::Texture& api::graphics::ResourceSprit::texture(Texture resource_id) {
  if (static_cast<int>(resource_id) < textures_.size()) {
    return textures_.at(static_cast<int>(resource_id));
  }
  return blank_texture_;
}

void api::graphics::ResourceSprit::LoadAllTextures() {
  blank_texture_ = sf::Texture();

  for (int i = 0; i < textures_.size(); i++) {
    if (!textures_.at(i).loadFromFile(path_ + names_textures_.at(i) + ".png"))
      std::cout << "Error loading " + path_ + names_textures_.at(i) +
                       ".png texture "
                << std::endl;
  }
}
