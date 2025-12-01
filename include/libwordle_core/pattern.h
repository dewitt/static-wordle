#pragma once
#include <cstdint>
#include <string_view>

namespace wordle {

// Computes the pattern (0-242) for a guess against a secret solution.
// Encoding: 0=Black, 1=Yellow, 2=Green.
// Result = sum(color[i] * 3^i) for i in [0..4]
uint8_t calc_pattern(std::string_view guess, std::string_view secret);

} // namespace wordle
