#include "libwordle_core/patterntable.h"
#include "libwordle_core/pattern.h"
#include <algorithm>
#include <cstring>
#include <future>
#include <iostream>
#include <thread>

namespace wordle {

struct PackedWord {
  uint8_t chars[5];
};

static PackedWord pack(const std::string &s) {
  PackedWord p;
  for (int i = 0; i < 5; ++i)
    p.chars[i] = s[i] - 'a';
  return p;
}

// Optimized pattern calc for packed words (avoids string overhead)
static uint8_t calc_pattern_fast(const PackedWord &guess,
                                 const PackedWord &secret) {
  uint8_t secret_counts[26] = {0};
  uint8_t guess_colors[5] = {0}; // 0=Black, 1=Yellow, 2=Green
  bool secret_matched[5] = {false};

  // 1. Green pass and count non-matched secret chars
  for (int i = 0; i < 5; ++i) {
    if (guess.chars[i] == secret.chars[i]) {
      guess_colors[i] = 2;
      secret_matched[i] = true;
    }
  }

  // Build histogram for secret (only for non-matched positions)
  for (int i = 0; i < 5; ++i) {
    if (!secret_matched[i]) {
      secret_counts[secret.chars[i]]++;
    }
  }

  // 2. Yellow pass
  for (int i = 0; i < 5; ++i) {
    if (guess_colors[i] == 2)
      continue;

    uint8_t c = guess.chars[i];
    if (secret_counts[c] > 0) {
      guess_colors[i] = 1;
      secret_counts[c]--;
    }
  }

  // Encode base-3
  uint8_t result = 0;
  int multiplier = 1;
  for (int i = 0; i < 5; ++i) {
    result += guess_colors[i] * multiplier;
    multiplier *= 3;
  }
  return result;
}

void PatternTable::generate(const std::vector<std::string> &guesses,
                            const std::vector<std::string> &solutions) {
  num_guesses_ = guesses.size();
  num_solutions_ = solutions.size();
  table_.resize(num_guesses_ * num_solutions_);

  // Pre-pack words
  std::vector<PackedWord> packed_guesses(num_guesses_);
  std::vector<PackedWord> packed_solutions(num_solutions_);

  for (size_t i = 0; i < num_guesses_; ++i)
    packed_guesses[i] = pack(guesses[i]);
  for (size_t i = 0; i < num_solutions_; ++i)
    packed_solutions[i] = pack(solutions[i]);

  unsigned int num_threads = std::thread::hardware_concurrency();
  if (num_threads == 0)
    num_threads = 4;

  std::vector<std::future<void>> futures;
  size_t batch_size = num_guesses_ / num_threads;

  for (unsigned int t = 0; t < num_threads; ++t) {
    size_t start = t * batch_size;
    size_t end = (t == num_threads - 1) ? num_guesses_ : start + batch_size;

    futures.push_back(
        std::async(std::launch::async,
                   [this, start, end, &packed_guesses, &packed_solutions]() {
                     for (size_t g = start; g < end; ++g) {
                       size_t row_offset = g * num_solutions_;
                       const auto &guess_word = packed_guesses[g];

                       for (size_t s = 0; s < num_solutions_; ++s) {
                         table_[row_offset + s] =
                             calc_pattern_fast(guess_word, packed_solutions[s]);
                       }
                     }
                   }));
  }

  for (auto &f : futures) {
    f.get();
  }
}

} // namespace wordle
