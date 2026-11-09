#include "procgen/noise.h"

#include <cmath>
#include <utility>

#include "procgen/random.h"

namespace core::procgen {

namespace {

// 8 directions de longueur 1 pour les gradients du Perlin.
constexpr float kDiag = 0.70710678f;
constexpr std::array<std::array<float, 2>, 8> kGradients = {{
    {1.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, -1.0f},
    {kDiag, kDiag}, {-kDiag, kDiag}, {kDiag, -kDiag}, {-kDiag, -kDiag},
}};

// Avec des gradients de longueur 1, le Perlin 2D reste entre -0.71 et 0.71.
// On multiplie par racine de 2 pour revenir a peu pres entre -1 et 1.
constexpr float kPerlinScale = 1.41421356f;

int FloorToInt(float v) { return static_cast<int>(std::floor(v)); }

}  // namespace

float Fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

float Lerp(float a, float b, float t) { return a + (b - a) * t; }

Noise::Noise(std::uint32_t seed) { Reseed(seed); }

void Noise::Reseed(std::uint32_t seed) {
  // Les nombres 0..255 dans l'ordre...
  for (int i = 0; i < 256; i++) {
    perm_[static_cast<std::size_t>(i)] = i;
  }
  // ... melanges avec la seed (melange de Fisher-Yates)...
  Random random(seed);
  for (int i = 255; i > 0; i--) {
    const int j = random.Int(0, i);
    std::swap(perm_[static_cast<std::size_t>(i)],
              perm_[static_cast<std::size_t>(j)]);
  }
  // ... puis recopies une 2e fois pour ne jamais deborder du tableau.
  for (std::size_t i = 0; i < 256; i++) {
    perm_[256 + i] = perm_[i];
  }
}

int Noise::Hash(int x, int y) const {
  const auto xi = static_cast<std::size_t>(x & 255);
  const auto yi = static_cast<std::size_t>(y & 255);
  return perm_[static_cast<std::size_t>(perm_[xi]) + yi];
}

float Noise::White(int x, int y) const {
  return static_cast<float>(Hash(x, y)) / 127.5f - 1.0f;
}

float Noise::Value(float x, float y) const {
  const int x0 = FloorToInt(x);
  const int y0 = FloorToInt(y);
  const float fx = x - static_cast<float>(x0);
  const float fy = y - static_cast<float>(y0);

  // Une valeur au hasard (entre -1 et 1) a chacun des 4 coins.
  const float v00 = White(x0, y0);
  const float v10 = White(x0 + 1, y0);
  const float v01 = White(x0, y0 + 1);
  const float v11 = White(x0 + 1, y0 + 1);

  const float u = Fade(fx);
  const float v = Fade(fy);
  return Lerp(Lerp(v00, v10, u), Lerp(v01, v11, u), v);
}

float Noise::Perlin(float x, float y) const {
  const int x0 = FloorToInt(x);
  const int y0 = FloorToInt(y);
  const float fx = x - static_cast<float>(x0);  // position dans la case (0..1)
  const float fy = y - static_cast<float>(y0);

  // Produit scalaire entre le gradient du coin et le vecteur coin -> point.
  const auto dot = [this](int cx, int cy, float dx, float dy) {
    const auto& g = kGradients[static_cast<std::size_t>(Hash(cx, cy) & 7)];
    return g[0] * dx + g[1] * dy;
  };

  const float n00 = dot(x0, y0, fx, fy);
  const float n10 = dot(x0 + 1, y0, fx - 1.0f, fy);
  const float n01 = dot(x0, y0 + 1, fx, fy - 1.0f);
  const float n11 = dot(x0 + 1, y0 + 1, fx - 1.0f, fy - 1.0f);

  const float u = Fade(fx);
  const float v = Fade(fy);
  return Lerp(Lerp(n00, n10, u), Lerp(n01, n11, u), v) * kPerlinScale;
}

float Noise::Fbm(float x, float y, int octaves, float persistence,
                 float lacunarity) const {
  float total = 0.0f;
  float amplitude = 1.0f;
  float frequency = 1.0f;
  float max = 0.0f;
  for (int i = 0; i < octaves; i++) {
    total += Perlin(x * frequency, y * frequency) * amplitude;
    max += amplitude;
    amplitude *= persistence;
    frequency *= lacunarity;
  }
  return max > 0.0f ? total / max : 0.0f;
}

float Noise::ValueFbm(float x, float y, int octaves, float persistence,
                      float lacunarity) const {
  float total = 0.0f;
  float amplitude = 1.0f;
  float frequency = 1.0f;
  float max = 0.0f;
  for (int i = 0; i < octaves; i++) {
    total += Value(x * frequency, y * frequency) * amplitude;
    max += amplitude;
    amplitude *= persistence;
    frequency *= lacunarity;
  }
  return max > 0.0f ? total / max : 0.0f;
}

}  // namespace core::procgen
