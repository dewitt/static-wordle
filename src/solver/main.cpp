#include "solver.h"
#include "libwordle_core/wordlist.h"
#include "libwordle_core/pattern.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <chrono>

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
    
    std::vector<std::string> positional_args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--solve" && i + 1 < argc) {
            target_word = argv[++i];
        } else {
            positional_args.push_back(arg);
        }
    }

    if (positional_args.empty()) {
        std::cerr << "Usage: " << argv[0] << " <solver_data.bin> [--solve <word>] [solutions.txt] [guesses.txt]" << std::endl;
        return 1;
    }

    bin_path = positional_args[0];
    if (positional_args.size() > 1) s_path = positional_args[1];
    if (positional_args.size() > 2) g_path = positional_args[2];
    
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
    
    int current_node = solver.get_root_index();

    if (!target_word.empty()) {
        // Non-interactive mode
        if (target_word.size() != 5) {
            std::cerr << "Error: Target word must be 5 characters." << std::endl;
            return 1;
        }
        std::cout << "Solving for target: " << target_word << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        int steps = 0;
        while (true) {
            steps++;
            const auto& node = solver.get_node(current_node);
            std::string guess = words.get_guesses()[node.guess_index];
            
            uint8_t pattern = wordle::calc_pattern(guess, target_word);
            std::string pat_str = pattern_to_string(pattern);
            
            std::cout << "Guess " << steps << ": " << guess << " (" << pat_str << ")" << std::endl;
            
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
            
            if (steps >= 10) { // Safety break
                std::cout << "Error: Loop detected or exceeded max steps." << std::endl;
                return 1;
            }
        }

    } else {
        // Interactive mode
        std::cout << "Wordle Solver Ready." << std::endl;
        
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
