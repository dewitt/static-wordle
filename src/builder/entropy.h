#pragma once
#include "libwordle_core/patterntable.h"
#include "state.h"

namespace wordle {

enum class HeuristicType { ENTROPY, MIN_EXPECTED };

struct HeuristicResult {
  double score; // Higher is better for Entropy, Lower is better for Expected
  int max_bucket;
};

HeuristicResult compute_heuristic(const SolverState &candidates, int guess_idx,
                                  const PatternTable &table,
                                  HeuristicType type);

} // namespace wordle
