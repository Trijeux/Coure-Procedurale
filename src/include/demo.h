#ifndef SRC_DEMO_H
#define SRC_DEMO_H

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <string>

namespace src::demo {

// Taille de la zone de dessin (la "carte"), a droite du panneau.
inline constexpr unsigned kMapWidth = 960;
inline constexpr unsigned kMapHeight = 720;

// Une demo = un algorithme. Le Game appelle ces fonctions a chaque image.
class Demo {
 public:
  virtual ~Demo() = default;

  [[nodiscard]] virtual const char* name() const = 0;
  // Nom court sans accent, pour les fichiers de capture (ex : "perlin").
  [[nodiscard]] virtual const char* file_tag() const = 0;

  // Le panneau de reglages (ImGui), a gauche.
  virtual void Gui() = 0;
  // Fenetres ImGui par-dessus la carte (ex : la liste de noms de Markov).
  virtual void OverlayGui(sf::Vector2f /*map_origin*/, sf::Vector2f /*map_size*/) {}

  virtual void Update(float /*dt*/) {}
  // Dessine dans une zone de kMapWidth x kMapHeight pixels.
  virtual void Draw(sf::RenderTarget& target) = 0;

  // Touche Espace : etape suivante (si la demo en a).
  virtual void NextStep() {}
  // Souris sur la carte (coordonnees dans la carte, en pixels).
  virtual void OnMouse(sf::Vector2f /*map_pos*/, bool /*left*/, bool /*right*/) {}

  // Faut-il capturer la fenetre entiere plutot que la carte ?
  [[nodiscard]] virtual bool draws_map() const { return true; }

  [[nodiscard]] std::uint32_t seed() const { return seed_; }
  void SetSeed(std::uint32_t seed) {
    seed_ = seed;
    Regenerate();
  }

 protected:
  // Relance l'algorithme avec la seed et les reglages actuels.
  virtual void Regenerate() = 0;

  // Champ "Seed" + bouton "Nouvelle seed". A appeler dans Gui().
  void SeedGui();

  std::uint32_t seed_ = 1234;
};

// Une seed au hasard (vrai hasard, pour le bouton "Nouvelle seed").
std::uint32_t RandomSeed();

}  // namespace src::demo

#endif  // SRC_DEMO_H
