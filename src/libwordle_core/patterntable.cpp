#include "libwordle_core/patterntable.h"
#include "libwordle_core/pattern.h"
#include <iostream>
#include <thread>
#include <future>
#include <algorithm>
#include <cstring>

namespace wordle {

void PatternTable::generate(const std::vector<std::string>& guesses, const std::vector<std::string>& solutions) {
    num_guesses_ = guesses.size();
    num_solutions_ = solutions.size();
    table_.resize(num_guesses_ * num_solutions_);

    // Pre-pack words
    std::vector<PackedWord> packed_guesses(num_guesses_);
    std::vector<PackedWord> packed_solutions(num_solutions_);
    
    for(size_t i=0; i<num_guesses_; ++i) packed_guesses[i] = pack_word(guesses[i]);
    for(size_t i=0; i<num_solutions_; ++i) packed_solutions[i] = pack_word(solutions[i]);

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    std::vector<std::future<void>> futures;
    size_t batch_size = num_guesses_ / num_threads;

    for (unsigned int t = 0; t < num_threads; ++t) {
        size_t start = t * batch_size;
        size_t end = (t == num_threads - 1) ? num_guesses_ : start + batch_size;

        futures.push_back(std::async(std::launch::async, [this, start, end, &packed_guesses, &packed_solutions]() {
            for (size_t g = start; g < end; ++g) {
                size_t row_offset = g * num_solutions_;
                const auto& guess_word = packed_guesses[g];
                
                for (size_t s = 0; s < num_solutions_; ++s) {
                    table_[row_offset + s] = calc_pattern(guess_word, packed_solutions[s]);
                }
            }
        }));
    }

    for (auto& f : futures) {
        f.get();
    }
}

} // namespace wordle