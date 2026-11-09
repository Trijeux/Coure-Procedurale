#ifndef CORE_PROCGEN_MARKOV_H
#define CORE_PROCGEN_MARKOV_H

#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "procgen/random.h"

namespace core::procgen {

// Chaine de Markov sur les lettres.
// Apprentissage : on compte quelle lettre suit chaque groupe de "order" lettres.
// Generation : on tire la lettre suivante selon ces comptes, jusqu'a la fin.
class MarkovChain {
 public:
  static constexpr char kStart = '^';  // debut de mot
  static constexpr char kEnd = '$';    // fin de mot

  explicit MarkovChain(int order = 2);

  // Apprend a partir d'une liste de mots (en minuscules, sans accents).
  void Train(const std::vector<std::string>& words);

  // Invente un mot. Il peut etre identique a un mot de la liste !
  std::string Generate(Random& random, int max_length = 20) const;

  // Les lettres qui suivent un contexte, avec leur nombre d'apparitions.
  // Le contexte doit faire exactement "order" lettres (avec des ^ au debut).
  [[nodiscard]] std::vector<std::pair<char, int>> Successors(
      const std::string& context) const;

  [[nodiscard]] int order() const { return order_; }
  [[nodiscard]] std::size_t context_count() const { return table_.size(); }

 private:
  int order_;
  // table_["ly"]['o'] = 3 : "o" apparait 3 fois apres "ly".
  std::unordered_map<std::string, std::map<char, int>> table_;
};

}  // namespace core::procgen

#endif  // CORE_PROCGEN_MARKOV_H
