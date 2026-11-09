#ifndef API_GRAPHICS_RESOURCE_MANAGER_H
#define API_GRAPHICS_RESOURCE_MANAGER_H

#include "graphics/resource_font.h"
#include "graphics/resource_sprit.h"
#include "sound/resource_sound.h"

namespace api::general::resource_manager {
void Setup();
sf::Texture& texture(graphics::ResourceSprit::Texture resource_id);
sf::SoundBuffer& sound(sound::ResourceSound::Sound resource_id);
sf::Font& font(graphics::ResourceFont::Font resource_id);
}  // namespace api::general::resource_manager

#endif  // API_GRAPHICS_RESOURCE_MANAGER_H
