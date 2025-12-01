#pragma once
#include <string_view>
#include <cstdint>
#include <string>

namespace wordle {

// Computes the pattern (0-242) for a guess against a secret solution.
// Encoding: 0=Black, 1=Yellow, 2=Green.
// Result = sum(color[i] * 3^i) for i in [0..4]
uint8_t calc_pattern(std::string_view guess, std::string_view secret);

struct PackedWord {
    uint8_t chars[5];
};

PackedWord pack_word(std::string_view s);

// Optimized calculation using pre-packed words (0-25 integers)
uint8_t calc_pattern(const PackedWord& guess, const PackedWord& secret);

} // namespace wordle
