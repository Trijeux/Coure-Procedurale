#include "sound/resource_sound.h"

#include <iostream>

void api::sound::ResourceSound::Setup() { LoadAllSounds(); }

sf::SoundBuffer& api::sound::ResourceSound::sound(Sound resource_id) {
  if (static_cast<int>(resource_id) < sounds_.size()) {
    return sounds_.at(static_cast<int>(resource_id));
  }
  return blank_sound_;
}

void api::sound::ResourceSound::LoadAllSounds() {
  blank_sound_ = sf::SoundBuffer();

  for (int i = 0; i < sounds_.size(); i++) {
    if (!sounds_.at(i).loadFromFile(path_ + names_sound_.at(i) + ".ogg"))
      std::cout << "Error loading " + path_ + names_sound_.at(i) + ".ogg sound "
                << std::endl;
  }
}
