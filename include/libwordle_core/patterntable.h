#pragma once
#include <vector>
#include <cstdint>
#include <string>

namespace wordle {

class PatternTable {
public:
    PatternTable() = default;

    // Computes the full matrix P[guess][solution].
    // This can take a few seconds for full lists.
    void generate(const std::vector<std::string>& guesses, const std::vector<std::string>& solutions);

    uint8_t get_pattern(size_t guess_idx, size_t sol_idx) const {
        return table_[guess_idx * num_solutions_ + sol_idx];
    }

    size_t num_guesses() const { return num_guesses_; }
    size_t num_solutions() const { return num_solutions_; }
    
    const std::vector<uint8_t>& get_raw_table() const { return table_; }

private:
    std::vector<uint8_t> table_;
    size_t num_guesses_ = 0;
    size_t num_solutions_ = 0;
};

} // namespace wordle
