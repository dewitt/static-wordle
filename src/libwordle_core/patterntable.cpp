#include "libwordle_core/patterntable.h"
#include "libwordle_core/pattern.h"
#include <iostream>

namespace wordle {

void PatternTable::generate(const std::vector<std::string>& guesses, const std::vector<std::string>& solutions) {
    num_guesses_ = guesses.size();
    num_solutions_ = solutions.size();
    table_.resize(num_guesses_ * num_solutions_);

    // Naive CPU generation
    // Loop order: For better cache locality if we access by guess then solution?
    // Access pattern is P[g][s]. 
    // Usually we iterate over guesses for a set of solutions.
    // So laying out as [guess][solution] (row-major) means for a fixed guess, solutions are contiguous.
    // That is optimal for entropy calc (SIMD over solutions).
    
    for (size_t g = 0; g < num_guesses_; ++g) {
        for (size_t s = 0; s < num_solutions_; ++s) {
            table_[g * num_solutions_ + s] = calc_pattern(guesses[g], solutions[s]);
        }
    }
}

} // namespace wordle
