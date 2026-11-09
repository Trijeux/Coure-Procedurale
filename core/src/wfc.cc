#include "procgen/wfc.h"

#include <bit>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>

namespace core::procgen {

namespace {

Wfc::Mask Bit(int tile) { return Wfc::Mask{1} << static_cast<unsigned>(tile); }

}  // namespace

void Wfc::SetTiles(std::vector<Tile> tiles) {
  tiles_ = std::move(tiles);
  compatible_.assign(tiles_.size(), 0);
}

void Wfc::SetWeight(int tile, float weight) {
  tiles_[static_cast<std::size_t>(tile)].weight = weight;
}

void Wfc::Allow(int a, int b) {
  compatible_[static_cast<std::size_t>(a)] |= Bit(b);
  compatible_[static_cast<std::size_t>(b)] |= Bit(a);
}

bool Wfc::Allowed(int a, int b) const {
  return (compatible_[static_cast<std::size_t>(a)] & Bit(b)) != 0;
}

Wfc::Mask Wfc::AllTiles() const {
  return tiles_.size() >= 32 ? std::numeric_limits<Mask>::max()
                             : (Mask{1} << tiles_.size()) - 1;
}

void Wfc::Start(int width, int height, std::uint32_t seed,
                std::vector<Mask> guide) {
  width_ = width;
  height_ = height;
  random_.Reseed(seed);
  contradictions_ = 0;
  guide_ = std::move(guide);
  guide_failed_ = false;
  if (!Reset()) {
    // Le guide se contredit lui-meme : on l'oublie.
    guide_failed_ = true;
    guide_.clear();
    Reset();
  }
}

void Wfc::SetPreferred(std::vector<int> preferred, float bias) {
  preferred_ = std::move(preferred);
  preferred_bias_ = bias;
}

bool Wfc::Reset() {
  // Au debut, chaque case peut tout etre : c'est la "superposition".
  const auto count = static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);
  cells_.assign(count, AllTiles());
  done_ = false;
  if (guide_.size() != count) {
    return true;
  }
  // Avec un guide : on applique les contraintes de depart, puis on propage.
  for (std::size_t i = 0; i < count; i++) {
    cells_[i] &= guide_[i];
    if (cells_[i] == 0) {
      return false;
    }
  }
  for (int i = 0; i < static_cast<int>(count); i++) {
    if (!Propagate(i)) {
      return false;
    }
  }
  return true;
}

int Wfc::OptionCount(int x, int y) const {
  return std::popcount(cells_[static_cast<std::size_t>(Index(x, y))]);
}

int Wfc::CollapsedTile(int x, int y) const {
  const Mask m = cells_[static_cast<std::size_t>(Index(x, y))];
  return std::popcount(m) == 1 ? std::countr_zero(m) : -1;
}

int Wfc::collapsed_count() const {
  int count = 0;
  for (const Mask m : cells_) {
    count += std::popcount(m) == 1 ? 1 : 0;
  }
  return count;
}

float Wfc::Entropy(Mask mask) const {
  // Entropie de Shannon avec les poids. Pour comprendre l'idee, on peut
  // remplacer tout ca par "le nombre de tuiles possibles" : ca marche aussi.
  float sum = 0.0f;
  float sum_log = 0.0f;
  for (std::size_t t = 0; t < tiles_.size(); t++) {
    if ((mask & Bit(static_cast<int>(t))) != 0) {
      const float w = tiles_[t].weight;
      if (w > 0.0f) {
        sum += w;
        sum_log += w * std::log(w);
      }
    }
  }
  return sum > 0.0f ? std::log(sum) - sum_log / sum : 0.0f;
}

Wfc::Status Wfc::Step() {
  if (done_) {
    return Status::kDone;
  }

  // 1. OBSERVER : la case non fixee avec la plus petite entropie.
  int best = -1;
  float best_entropy = std::numeric_limits<float>::max();
  for (std::size_t i = 0; i < cells_.size(); i++) {
    if (std::popcount(cells_[i]) <= 1) {
      continue;  // deja fixee
    }
    // Un tout petit peu de hasard pour departager les egalites.
    const float e = Entropy(cells_[i]) + random_.Float() * 0.001f;
    if (e < best_entropy) {
      best_entropy = e;
      best = static_cast<int>(i);
    }
  }
  if (best == -1) {
    done_ = true;  // tout est fixe : fini !
    return Status::kDone;
  }

  // 2. FIXER : tirage pondere parmi les tuiles encore possibles.
  const Mask options = cells_[static_cast<std::size_t>(best)];
  const int preferred = static_cast<std::size_t>(best) < preferred_.size()
                            ? preferred_[static_cast<std::size_t>(best)]
                            : -1;
  const auto weight = [&](std::size_t t) {
    const float w = tiles_[t].weight;
    return static_cast<int>(t) == preferred ? w * preferred_bias_ : w;
  };
  float total = 0.0f;
  for (std::size_t t = 0; t < tiles_.size(); t++) {
    if ((options & Bit(static_cast<int>(t))) != 0) {
      total += weight(t);
    }
  }
  float pick = random_.Float() * total;
  int chosen = std::countr_zero(options);  // par defaut : la premiere possible
  for (std::size_t t = 0; t < tiles_.size(); t++) {
    if ((options & Bit(static_cast<int>(t))) == 0) {
      continue;
    }
    pick -= weight(t);
    if (pick <= 0.0f) {
      chosen = static_cast<int>(t);
      break;
    }
  }
  cells_[static_cast<std::size_t>(best)] = Bit(chosen);

  // 3. PROPAGER
  if (!Propagate(best)) {
    contradictions_++;
    Reset();  // le plus simple : on recommence tout (le guide reste valable)
    return Status::kContradiction;
  }
  return Status::kRunning;
}

bool Wfc::Force(int x, int y, int tile) {
  const auto i = static_cast<std::size_t>(Index(x, y));
  if ((cells_[i] & Bit(tile)) == 0) {
    return false;  // les voisines interdisent deja cette tuile
  }
  const Mask before = cells_[i];
  const std::vector<Mask> backup = cells_;
  cells_[i] = Bit(tile);
  if (before != cells_[i] && !Propagate(Index(x, y))) {
    cells_ = backup;  // on annule
    return false;
  }
  done_ = false;
  return true;
}

bool Wfc::Propagate(int start) {
  std::vector<int> todo = {start};
  while (!todo.empty()) {
    const int c = todo.back();
    todo.pop_back();
    const int cx = c % width_;
    const int cy = c / width_;

    // Toutes les tuiles autorisees a cote d'au moins une tuile possible de c.
    Mask allowed = 0;
    const Mask options = cells_[static_cast<std::size_t>(c)];
    for (std::size_t t = 0; t < tiles_.size(); t++) {
      if ((options & Bit(static_cast<int>(t))) != 0) {
        allowed |= compatible_[t];
      }
    }

    const int nx[4] = {cx, cx + 1, cx, cx - 1};
    const int ny[4] = {cy - 1, cy, cy + 1, cy};
    for (int d = 0; d < 4; d++) {
      if (nx[d] < 0 || ny[d] < 0 || nx[d] >= width_ || ny[d] >= height_) {
        continue;
      }
      const auto n = static_cast<std::size_t>(Index(nx[d], ny[d]));
      const Mask reduced = cells_[n] & allowed;
      if (reduced == cells_[n]) {
        continue;  // rien n'a change : inutile d'aller plus loin
      }
      cells_[n] = reduced;
      if (reduced == 0) {
        return false;  // contradiction : plus aucune tuile possible ici
      }
      todo.push_back(Index(nx[d], ny[d]));  // ses voisines doivent verifier
    }
  }
  return true;
}

}  // namespace core::procgen
