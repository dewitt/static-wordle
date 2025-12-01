#include "solver.h"
#include "libwordle_core/wordlist.h"
#include "libwordle_core/pattern.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <chrono>
#include <numeric>

uint8_t parse_feedback(const std::string& input) {
    if (input.size() != 5) return 255;
    uint8_t res = 0;
    int mult = 1;
    for (char c : input) {
        int val = 0;
        if (c == 'B' || c == 'b') val = 0;
        else if (c == 'Y' || c == 'y') val = 1;
        else if (c == 'G' || c == 'g') val = 2;
        else return 255;
        
        res += val * mult;
        mult *= 3;
    }
    return res;
}

std::string pattern_to_string(uint8_t p) {
    std::string res = "     ";
    for (int i = 0; i < 5; ++i) {
        int val = p % 3;
        p /= 3;
        if (val == 0) res[i] = 'B';
        else if (val == 1) res[i] = 'Y';
        else if (val == 2) res[i] = 'G';
    }
    return res;
}

int main(int argc, char** argv) {
    std::string bin_path;
    std::string s_path = "data/solutions.txt";
    std::string g_path = "data/guesses.txt";
    std::string target_word;
    std::string single_list_path;
    bool benchmark_mode = false;
    
    std::vector<std::string> positional_args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--solve" && i + 1 < argc) {
            target_word = argv[++i];
        } else if (arg == "--single-list" && i + 1 < argc) {
            single_list_path = argv[++i];
        } else if (arg == "--benchmark") {
            benchmark_mode = true;
        } else {
            positional_args.push_back(arg);
        }
    }

    if (positional_args.empty()) {
        std::cerr << "Usage: " << argv[0] << " <solver_data.bin> [--solve <word> | --benchmark] [--single-list <path>] [solutions.txt] [guesses.txt]" << std::endl;
        return 1;
    }

    bin_path = positional_args[0];
    if (positional_args.size() > 1) s_path = positional_args[1];
    if (positional_args.size() > 2) g_path = positional_args[2];
    
    if (!single_list_path.empty()) {
        s_path = single_list_path;
        g_path = single_list_path;
    }

    wordle::WordList words;
    if (!words.load(s_path, g_path)) {
        std::cerr << "Failed to load word lists" << std::endl;
        return 1;
    }
    
    wordle::Solver solver;
    if (!solver.load(bin_path)) {
        std::cerr << "Failed to load solver data" << std::endl;
        return 1;
    }
    
    if (solver.get_checksum() != words.get_checksum()) {
        std::cerr << "Warning: Checksum mismatch! Word lists might be different." << std::endl;
    }
    
    // Pre-pack all guesses for fast simulation
    std::vector<wordle::PackedWord> packed_guesses(words.get_guesses().size());
    for(size_t i=0; i<words.get_guesses().size(); ++i) {
        packed_guesses[i] = wordle::pack_word(words.get_guesses()[i]);
    }

    int root_node = solver.get_root_index();

    if (benchmark_mode) {
        std::cout << "Benchmarking against all " << words.get_solutions().size() << " solutions..." << std::endl;
        auto start_total = std::chrono::high_resolution_clock::now();
        
        long long total_guesses = 0;
        
        for (const auto& sol_str : words.get_solutions()) {
            wordle::PackedWord target = wordle::pack_word(sol_str);
            int current_node = root_node;
            int steps = 0;
            while (true) {
                steps++;
                const auto& node = solver.get_node(current_node);
                // Use packed guess directly? Node has guess_index.
                // We need the guess word to calc pattern.
                const auto& guess_packed = packed_guesses[node.guess_index];
                
                uint8_t pattern = wordle::calc_pattern(guess_packed, target);
                
                if (pattern == 242) {
                    break;
                }
                
                int next = solver.get_next_node(current_node, pattern);
                current_node = next;
            }
            total_guesses += steps;
        }
        
        auto end_total = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_total - start_total).count();
        
        std::cout << "Solved " << words.get_solutions().size() << " games in " << duration / 1000.0 << " ms." << std::endl;
        std::cout << "Average time per game: " << duration / (double)words.get_solutions().size() << " µs." << std::endl;
        std::cout << "Average guesses: " << (double)total_guesses / words.get_solutions().size() << std::endl;

    } else if (!target_word.empty()) {
        // Non-interactive mode
        if (target_word.size() != 5) {
            std::cerr << "Error: Target word must be 5 characters." << std::endl;
            return 1;
        }
        std::cout << "Solving for target: " << target_word << std::endl;
        
        wordle::PackedWord target = wordle::pack_word(target_word);
        
        auto start_time = std::chrono::high_resolution_clock::now();
        int current_node = root_node;
        int steps = 0;
        
        while (true) {
            steps++;
            const auto& node = solver.get_node(current_node);
            // Get string for display
            std::string guess_str = words.get_guesses()[node.guess_index];
            const auto& guess_packed = packed_guesses[node.guess_index];
            
            uint8_t pattern = wordle::calc_pattern(guess_packed, target);
            std::string pat_str = pattern_to_string(pattern);
            
            std::cout << "Guess " << steps << ": " << guess_str << " (" << pat_str << ")" << std::endl;
            
            if (pattern == 242) {
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
                std::cout << "Solved in " << steps << " guesses! (" << duration << " µs)" << std::endl;
                break;
            }
            
            int next = solver.get_next_node(current_node, pattern);
            if (next == -1 || next == (int)0xFFFFFFFF) {
                std::cout << "Error: Impossible state reached or word not in tree." << std::endl;
                return 1;
            }
            current_node = next;
            
            if (steps >= 10) { 
                std::cout << "Error: Loop detected or exceeded max steps." << std::endl;
                return 1;
            }
        }

    } else {
        // Interactive mode (unchanged)
        std::cout << "Wordle Solver Ready." << std::endl;
        int current_node = root_node;
        while (true) {
            const auto& node = solver.get_node(current_node);
            std::string guess = words.get_guesses()[node.guess_index];
            std::cout << "Suggestion: " << guess << std::endl;
            
            std::string input;
            std::cout << "Enter Feedback (GYB): ";
            std::cin >> input;
            
            if (input == "exit" || input == "quit") break;
            
            uint8_t pattern = parse_feedback(input);
            if (pattern == 255) {
                std::cout << "Invalid input. Use 5 chars G/Y/B. Example: GYBBG" << std::endl;
                continue;
            }
            
            if (pattern == 242) { // GGGGG
                std::cout << "Solved! The word was " << guess << "." << std::endl;
                break;
            }
            
            int next = solver.get_next_node(current_node, pattern);
            if (next == -1 || next == (int)0xFFFFFFFF) {
                std::cout << "Impossible pattern (or not found in tree)." << std::endl;
                continue;
            }
            
            current_node = next;
        }
    }
    
    return 0;
}