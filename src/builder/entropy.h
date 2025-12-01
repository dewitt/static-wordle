#pragma once
#include "libwordle_core/patterntable.h"
#include "state.h"

namespace wordle {

struct HeuristicResult {
  double entropy;
  int max_bucket;
};

HeuristicResult compute_heuristic(const SolverState &candidates, int guess_idx,
                                  const PatternTable &table);

} // namespace wordle
