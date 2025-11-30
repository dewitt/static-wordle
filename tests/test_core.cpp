#include "libwordle_core/pattern.h"
#include <cassert>
#include <iostream>

using namespace wordle;

void test_pattern() {
    // "SALET" vs "SALET" -> 22222 -> 242
    // 2 + 6 + 18 + 54 + 162 = 242
    assert(calc_pattern("salet", "salet") == 242);

    // "ABCDE" vs "FGHIJ" -> 00000 -> 0
    assert(calc_pattern("abcde", "fghij") == 0);

    // Secret: ABBEY, Guess: BABES
    // Green: ..GG.
    // Yellow: YY... (First B matches second B in ABBEY, A matches A)
    // Result: 11220 (base 3) -> 1 + 3 + 18 + 54 = 76
    int p = calc_pattern("babes", "abbey");
    // std::cout << "babes vs abbey: " << p << std::endl;
    assert(p == 76);
    
    // Secret: SIGHT, Guess: NIGHT
    // Green: .IGHT (02222)
    // Yellow: N vs S -> Black
    // Result: 02222 -> 0 + 6 + 18 + 54 + 162 = 240
    assert(calc_pattern("night", "sight") == 240);
}

int main() {
    test_pattern();
    std::cout << "All core tests passed." << std::endl;
    return 0;
}
