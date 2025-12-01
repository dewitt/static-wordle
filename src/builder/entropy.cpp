#include "entropy.h"
#include <array>
#include <cmath>
#include <mutex>
#include <vector>

namespace wordle {

// Precomputed table for x * log2(x)
static std::vector<double> LOG_TABLE;
// Precomputed table for Estimated Guesses E(n)
static std::vector<double> EXPECTED_TABLE;
static std::once_flag tables_flag;

void init_tables() {
  // Log Table
  LOG_TABLE.resize(13000); // Support full dictionary size just in case
  LOG_TABLE[0] = 0.0;
  for (int i = 1; i < 13000; ++i) {
    LOG_TABLE[i] = i * std::log2(static_cast<double>(i));
  }

  // Expected Guesses Table (Approximation)
  // E(1) = 0
  // E(2) = 1 (1 guess to pick the right one)
  // E(3) = (1/3)*1 + (2/3)*2 = 1.66? No.
  // E(n) approximation:
  // It takes roughly log_base(n) steps.
  // Let's use a sigmoid-like or log fit based on observations.
  EXPECTED_TABLE.resize(13000);
  EXPECTED_TABLE[0] = 0;
  EXPECTED_TABLE[1] = 0;
  EXPECTED_TABLE[2] = 1.0;
  for (int i = 3; i < 13000; ++i) {
    // Simple log model: log2(n) * scaling
    EXPECTED_TABLE[i] = std::log2(static_cast<double>(i)) * 1.5;
    // This is a rough heuristic to minimize tree depth
  }
}

HeuristicResult compute_heuristic(const SolverState &candidates, int guess_idx,
                                  const PatternTable &table,
                                  HeuristicType type) {
  std::call_once(tables_flag, init_tables);

  std::array<int, 243> counts = {0};

  const auto &words = candidates.get_words();
  size_t count = 0;

  const uint8_t *pattern_row =
      table.get_raw_table().data() + (guess_idx * table.num_solutions());

  for (size_t i = 0; i < words.size(); ++i) {
    uint64_t w = words[i];
    if (w == 0)
      continue;

    size_t base_idx = i * 64;

    while (w) {
      int bit = __builtin_ctzll(w);
      size_t sol_idx = base_idx + bit;

      counts[pattern_row[sol_idx]]++;
      count++;

      w &= (w - 1);
    }
  }

  double total = static_cast<double>(count);
  if (total == 0)
    return {0.0, 0};

  int max_bucket = 0;
  double score = 0.0;

  if (type == HeuristicType::ENTROPY) {
    // Entropy = log2(N) - (1/N) * sum(n * log2(n))
    double sum_n_log_n = 0.0;

    for (int c : counts) {
      if (c > 0) {
        sum_n_log_n += LOG_TABLE[c];
        if (c > max_bucket)
          max_bucket = c;
      }
    }
    score = std::log2(total) - (sum_n_log_n / total);
  } else {
    // Min Expected Guesses
    // Cost = 1 + Sum( (n/N) * E(n) )
    // We want to MINIMIZE this.
    double expected_sum = 0.0;
    for (int c : counts) {
      if (c > 0) {
        expected_sum += (static_cast<double>(c) / total) * EXPECTED_TABLE[c];
        if (c > max_bucket)
          max_bucket = c;
      }
    }
    score = 1.0 + expected_sum;
  }

  return {score, max_bucket};
}

} // namespace wordle