#ifndef SRC_DEMOS_MARKOV_DEMO_H
#define SRC_DEMOS_MARKOV_DEMO_H

#include <set>
#include <string>
#include <vector>

#include "demo.h"
#include "procgen/markov.h"

namespace src::demo {

// 0. Chaine de Markov : inventer des noms a partir d'une liste d'exemples.
// Les listes sont des fichiers texte dans resources/markov/ (un mot par ligne) :
// ajoutez le votre, il apparait tout seul dans la liste.
class MarkovDemo final : public Demo {
 public:
  MarkovDemo();

  [[nodiscard]] const char* name() const override { return "0. Chaine de Markov"; }
  [[nodiscard]] const char* file_tag() const override { return "markov"; }
  [[nodiscard]] bool draws_map() const override { return false; }

  void Gui() override;
  void OverlayGui(sf::Vector2f map_origin, sf::Vector2f map_size) override;
  void Draw(sf::RenderTarget& target) override;
  void NextStep() override { SetSeed(RandomSeed()); }

 protected:
  void Regenerate() override;

 private:
  struct Corpus {
    std::string name;
    std::vector<std::string> words;
  };
  struct Result {
    std::string word;
    bool copy = false;  // le mot existe deja dans la liste
  };

  void LoadCorpora();
  void Train();

  std::vector<Corpus> corpora_;
  int corpus_ = 0;
  int order_ = 2;
  int count_ = 30;
  int min_length_ = 3;
  int max_length_ = 12;
  core::procgen::MarkovChain chain_;
  std::set<std::string> known_;
  std::vector<Result> results_;
  int copies_ = 0;
  char context_[16] = "o";
};

}  // namespace src::demo

#endif  // SRC_DEMOS_MARKOV_DEMO_H
