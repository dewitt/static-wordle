#include "libwordle_core/wordlist.h"
#include "libwordle_core/patterntable.h"
#include "builder/state.h"
#include "builder/entropy.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>

struct ScoredWord {
    std::string word;
    double entropy;
};

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <solutions.txt> <guesses.txt> [top_n]" << std::endl;
        return 1;
    }

    std::string s_path = argv[1];
    std::string g_path = argv[2];
    int top_n = 100;
    if (argc > 3) top_n = std::stoi(argv[3]);

    wordle::WordList words;
    if (!words.load(s_path, g_path)) return 1;

    std::cout << "Generating Pattern Table..." << std::endl;
    wordle::PatternTable table;
    table.generate(words.get_guesses(), words.get_solutions());

    std::cout << "Ranking " << words.get_guesses().size() << " openers by entropy..." << std::endl;
    
    // Initial state: All solutions active
    wordle::SolverState all_solutions(words.get_solutions().size());
    for (size_t i = 0; i < words.get_solutions().size(); ++i) {
        all_solutions.set(i);
    }

    std::vector<ScoredWord> results;
    results.reserve(words.get_guesses().size());

    auto start = std::chrono::high_resolution_clock::now();

    // We can use the existing compute_heuristic function
    // It's already optimized.
    const auto& guesses = words.get_guesses();
    for (size_t i = 0; i < guesses.size(); ++i) {
        auto h = wordle::compute_heuristic(all_solutions, i, table);
        results.push_back({guesses[i], h.entropy});
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Ranking calculated in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    // Sort descending by entropy
    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        return a.entropy > b.entropy;
    });

    std::cout << "\nTop " << top_n << " Openers by Entropy:\n";
    std::cout << "--------------------------------\n";
    for (int i = 0; i < std::min((int)results.size(), top_n); ++i) {
        std::cout << std::fixed << std::setprecision(5) << results[i].word << " " << results[i].entropy << std::endl;
    }

    return 0;
}
