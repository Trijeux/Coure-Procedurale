#include "procgen/markov.h"

namespace core::procgen {

MarkovChain::MarkovChain(int order) : order_(order < 1 ? 1 : order) {}

void MarkovChain::Train(const std::vector<std::string>& words) {
  table_.clear();
  const auto order = static_cast<std::size_t>(order_);

  for (const std::string& word : words) {
    if (word.empty()) {
      continue;
    }
    // "lyon" a l'ordre 2 -> "^^lyon$"
    const std::string padded = std::string(order, kStart) + word + kEnd;

    for (std::size_t i = 0; i + order < padded.size(); i++) {
      const std::string context = padded.substr(i, order);
      const char next = padded[i + order];
      table_[context][next]++;
    }
  }
}

std::string MarkovChain::Generate(Random& random, int max_length) const {
  std::string context(static_cast<std::size_t>(order_), kStart);
  std::string word;

  while (static_cast<int>(word.size()) < max_length) {
    const auto found = table_.find(context);
    if (found == table_.end()) {
      break;  // contexte jamais vu (ne devrait pas arriver)
    }

    // Tirage pondere : plus une lettre a ete vue, plus elle a de chances.
    int total = 0;
    for (const auto& [letter, count] : found->second) {
      total += count;
    }
    int pick = random.Int(1, total);
    char next = kEnd;
    for (const auto& [letter, count] : found->second) {
      pick -= count;
      if (pick <= 0) {
        next = letter;
        break;
      }
    }

    if (next == kEnd) {
      break;
    }
    word += next;
    context = context.substr(1) + next;  // on fait glisser la fenetre
  }
  return word;
}

std::vector<std::pair<char, int>> MarkovChain::Successors(
    const std::string& context) const {
  std::vector<std::pair<char, int>> result;
  const auto found = table_.find(context);
  if (found != table_.end()) {
    result.assign(found->second.begin(), found->second.end());
  }
  return result;
}

}  // namespace core::procgen
