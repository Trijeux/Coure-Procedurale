#ifndef CORE_PROCGEN_NOISE_H
#define CORE_PROCGEN_NOISE_H

#include <array>
#include <cstdint>

namespace core::procgen {

// Les 3 bruits du cours, du pire au meilleur :
//   - WhiteNoise : un nombre au hasard par point (aucune forme)
//   - ValueNoise : des valeurs au hasard sur la grille, lissees entre elles
//   - Perlin     : des directions (gradients) au hasard sur la grille
//
// Ils renvoient tous un nombre entre -1 et 1 environ.
// Tous utilisent la meme table de permutation, melangee avec la seed.
class Noise {
 public:
  explicit Noise(std::uint32_t seed = 0);

  void Reseed(std::uint32_t seed);

  [[nodiscard]] float White(int x, int y) const;
  [[nodiscard]] float Value(float x, float y) const;
  [[nodiscard]] float Perlin(float x, float y) const;

  // Plusieurs couches (octaves) de Perlin additionnees : le "fBm".
  [[nodiscard]] float Fbm(float x, float y, int octaves, float persistence,
                          float lacunarity) const;

  // Pareil mais avec le Value Noise, pour comparer.
  [[nodiscard]] float ValueFbm(float x, float y, int octaves,
                               float persistence, float lacunarity) const;

 private:
  // Un nombre entre 0 et 255 unique pour chaque coin (x, y) de la grille.
  [[nodiscard]] int Hash(int x, int y) const;

  std::array<int, 512> perm_{};
};

// Courbe d'adoucissement de Ken Perlin : 6t^5 - 15t^4 + 10t^3.
float Fade(float t);
float Lerp(float a, float b, float t);

}  // namespace core::procgen

#endif  // CORE_PROCGEN_NOISE_H
