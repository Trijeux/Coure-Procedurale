#include "graphics/resource_font.h"
#include "graphics/resource_sprit.h"
#include "sound/resource_sound.h"

namespace api::general::resource_manager {
namespace {
graphics::ResourceSprit resource_sprit;
sound::ResourceSound resource_sound;
graphics::ResourceFont resource_font;
}  // namespace

void Setup() {
  static bool setup = false;

  if (setup) {
    return;
  }
  setup = true;

  resource_sprit.Setup();
  resource_sound.Setup();
  resource_font.Setup();
}

sf::Texture& texture(const graphics::ResourceSprit::Texture resource_id) {
  return resource_sprit.texture(resource_id);
}

sf::SoundBuffer& sound(const sound::ResourceSound::Sound resource_id) {
  return resource_sound.sound(resource_id);
}

sf::Font& font(const graphics::ResourceFont::Font resource_id) {
  return resource_font.font(resource_id);
}
}  // namespace api::general::resource_manager
