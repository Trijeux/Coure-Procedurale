#ifndef CORE_PROCGEN_RANDOM_H
#define CORE_PROCGEN_RANDOM_H

#include <cstdint>
#include <random>

namespace core::procgen {

// Le hasard controle : tout le projet tire ses nombres ici.
// Meme seed = meme suite de nombres = meme monde.
//
// Attention : std::uniform_int_distribution, std::shuffle... ne donnent PAS
// les memes resultats d'un compilateur a l'autre (MSVC, GCC, Clang).
// Seul le moteur std::mt19937 est garanti identique partout.
// On fait donc nos propres tirages a partir du moteur : la seed 1234 donne
// la meme carte sur toutes les machines.
class Random {
 public:
  explicit Random(std::uint32_t seed = 0) : engine_(seed) {}

  void Reseed(std::uint32_t seed) { engine_.seed(seed); }

  // Un nombre brut entre 0 et 2^32 - 1.
  std::uint32_t Next() { return static_cast<std::uint32_t>(engine_()); }

  // Un entier entre min et max (inclus).
  int Int(int min, int max) {
    if (max <= min) {
      return min;
    }
    const auto range = static_cast<std::uint32_t>(max - min) + 1u;
    return min + static_cast<int>(Next() % range);
  }

  // Un reel entre 0 (inclus) et 1 (exclu).
  float Float() {
    return static_cast<float>(Next() >> 8) * (1.0f / 16777216.0f);
  }

  // Vrai avec la probabilite donnee (0.3f = 30 % de chances).
  bool Chance(float probability) { return Float() < probability; }

 private:
  std::mt19937 engine_;
};

}  // namespace core::procgen

#endif  // CORE_PROCGEN_RANDOM_H
