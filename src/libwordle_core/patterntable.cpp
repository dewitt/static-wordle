#include "libwordle_core/patterntable.h"
#include "libwordle_core/pattern.h"
#include <iostream>
#include <thread>
#include <future>
#include <algorithm>

namespace wordle {

void PatternTable::generate(const std::vector<std::string>& guesses, const std::vector<std::string>& solutions) {
    num_guesses_ = guesses.size();
    num_solutions_ = solutions.size();
    table_.resize(num_guesses_ * num_solutions_);

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    std::vector<std::future<void>> futures;
    size_t batch_size = num_guesses_ / num_threads;

    // Parallelize over guesses
    for (unsigned int t = 0; t < num_threads; ++t) {
        size_t start = t * batch_size;
        size_t end = (t == num_threads - 1) ? num_guesses_ : start + batch_size;

        futures.push_back(std::async(std::launch::async, [this, start, end, &guesses, &solutions]() {
            for (size_t g = start; g < end; ++g) {
                size_t row_offset = g * num_solutions_;
                const auto& guess_word = guesses[g];
                
                // Inner loop over solutions
                for (size_t s = 0; s < num_solutions_; ++s) {
                    table_[row_offset + s] = calc_pattern(guess_word, solutions[s]);
                }
            }
        }));
    }

    // Wait for all threads
    for (auto& f : futures) {
        f.get();
    }
}

} // namespace wordle