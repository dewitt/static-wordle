#include "entropy.h"
#include <cmath>
#include <array>
#include <vector>

namespace wordle {

HeuristicResult compute_heuristic(const SolverState& candidates, int guess_idx, const PatternTable& table) {
    std::array<int, 243> counts = {0};
    
    // Optimized iteration: avoid get_active_indices() allocation
    const auto& words = candidates.get_words();
    size_t count = 0;
    
    // We can assume table is row-major: table[guess * num_sol + sol]
    // So we get a pointer to the start of the row for this guess.
    const uint8_t* pattern_row = table.get_raw_table().data() + (guess_idx * table.num_solutions());
    
    for (size_t i = 0; i < words.size(); ++i) {
        uint64_t w = words[i];
        if (w == 0) continue;
        
        size_t base_idx = i * 64;
        
        // Iterate set bits
        while (w) {
            int bit = __builtin_ctzll(w);
            size_t sol_idx = base_idx + bit;
            
            // pattern_row[sol_idx] is faster than get_pattern call overhead
            counts[pattern_row[sol_idx]]++;
            count++;
            
            w &= (w - 1); // Clear lowest set bit
        }
    }
    
    double total = static_cast<double>(count);
    if (total == 0) return {0.0, 0};

    double entropy = 0.0;
    int max_bucket = 0;
    
    // Precompute log2?
    // Not strictly necessary as loop is small (243), but log2 is slow.
    // However, most buckets are 0.
    
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