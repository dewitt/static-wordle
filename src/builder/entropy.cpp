#include "entropy.h"
#include <cmath>
#include <array>
#include <vector>

namespace wordle {

HeuristicResult compute_heuristic(const SolverState& candidates, int guess_idx, const PatternTable& table) {
    std::array<int, 243> counts = {0};
    
    auto indices = candidates.get_active_indices();
    double total = static_cast<double>(indices.size());
    
    for (int sol_idx : indices) {
        uint8_t p = table.get_pattern(guess_idx, sol_idx);
        counts[p]++;
    }
    
    double entropy = 0.0;
    int max_bucket = 0;
    
    for (int c : counts) {
        if (c > 0) {
            double p = c / total;
            entropy -= p * std::log2(p);
            if (c > max_bucket) max_bucket = c;
        }
    }
    
    return {entropy, max_bucket};
}

} // namespace wordle
