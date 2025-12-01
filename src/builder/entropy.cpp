#include "entropy.h"
#include <array>
#include <cmath>
#include <mutex>
#include <vector>

namespace wordle {

// Precomputed table for x * log2(x)
// Max bucket size is 2315 (all solutions).
static std::vector<double> LOG_TABLE;
static std::once_flag log_table_flag;

void init_log_table() {
  LOG_TABLE.resize(2316);
  LOG_TABLE[0] = 0.0;
  for (int i = 1; i <= 2315; ++i) {
    LOG_TABLE[i] = i * std::log2(static_cast<double>(i));
  }
}

HeuristicResult compute_heuristic(const SolverState &candidates, int guess_idx,
                                  const PatternTable &table) {
  std::call_once(log_table_flag, init_log_table);

  std::array<int, 243> counts = {0};

  // Optimized iteration: avoid get_active_indices() allocation
  const auto &words = candidates.get_words();
  size_t count = 0;

  // We can assume table is row-major: table[guess * num_sol + sol]
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

  // Entropy = log2(N) - (1/N) * sum(n * log2(n))
  double sum_n_log_n = 0.0;
  int max_bucket = 0;

  for (int c : counts) {
    if (c > 0) {
      sum_n_log_n += LOG_TABLE[c];
      if (c > max_bucket)
        max_bucket = c;
    }
  }

  double entropy = std::log2(total) - (sum_n_log_n / total);

  return {entropy, max_bucket};
}

} // namespace wordle
