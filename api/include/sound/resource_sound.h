#ifndef API_SOUND_RESOURCE_SOUND_H
#define API_SOUND_RESOURCE_SOUND_H

#include <SFML/Audio/SoundBuffer.hpp>
#include <array>

namespace api::sound {
class ResourceSound {
 public:
  enum class Sound { kMax };

  void Setup();

  sf::SoundBuffer& sound(Sound resource_id);

 protected:
 private:
  const std::array<std::string, static_cast<int>(Sound::kMax)> names_sound_{};

  std::string path_ = "resources/sound/";

  std::array<sf::SoundBuffer, static_cast<int>(Sound::kMax)> sounds_;

  sf::SoundBuffer blank_sound_;

  void LoadAllSounds();
};
}  // namespace api::sound

#endif  // API_SOUND_RESOURCE_SOUND_H
