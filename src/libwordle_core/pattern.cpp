#include "libwordle_core/pattern.h"
#include <vector>

namespace wordle {

uint8_t calc_pattern(std::string_view guess, std::string_view secret) {
    int secret_counts[26] = {0};
    uint8_t guess_colors[5] = {0}; // 0=Black
    bool secret_matched[5] = {false}; // Track which secret chars are consumed by Green

    // 1. Green pass
    for (int i = 0; i < 5; ++i) {
        if (guess[i] == secret[i]) {
            guess_colors[i] = 2;
            secret_matched[i] = true;
        } else {
             // Only count available chars for Yellow pass
        }
    }

    // Populate counts for non-green secret chars
    for (int i = 0; i < 5; ++i) {
        if (!secret_matched[i]) {
            secret_counts[secret[i] - 'a']++;
        }
    }

    // 2. Yellow pass
    for (int i = 0; i < 5; ++i) {
        if (guess_colors[i] == 2) continue; // Already green

        int char_idx = guess[i] - 'a';
        if (secret_counts[char_idx] > 0) {
            guess_colors[i] = 1;
            secret_counts[char_idx]--;
        }
    }

    // Encode
    uint8_t result = 0;
    int multiplier = 1;
    for (int i = 0; i < 5; ++i) {
        result += guess_colors[i] * multiplier;
        multiplier *= 3;
    }

    return result;
}

} // namespace wordle
