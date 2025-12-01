#include "builder.h"
#include "entropy.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <cmath>

namespace wordle {

static uint32_t compute_mask(const std::string& w) {
    uint32_t mask = 0;
    for (char c : w) {
        mask |= (1 << (c - 'a'));
    }
    return mask;
}

Builder::Builder(const WordList& words, const PatternTable& table, const std::string& start_word)
    : words_(words), table_(table), start_word_(start_word) {
    
    // Precompute solution -> guess mapping
    solution_to_guess_.resize(words_.get_solutions().size());
    const auto& guesses = words_.get_guesses();
    for (size_t i = 0; i < words_.get_solutions().size(); ++i) {
        const std::string& sol = words_.get_solutions()[i];
        auto it = std::lower_bound(guesses.begin(), guesses.end(), sol);
        if (it != guesses.end() && *it == sol) {
            solution_to_guess_[i] = std::distance(guesses.begin(), it);
        } else {
            std::cerr << "Error: Solution " << sol << " not found in guesses!" << std::endl;
        }
    }
    
    // Precompute masks
    guess_masks_.resize(guesses.size());
    for(size_t i=0; i<guesses.size(); ++i) guess_masks_[i] = compute_mask(guesses[i]);
    
    solution_masks_.resize(words_.get_solutions().size());
    for(size_t i=0; i<words_.get_solutions().size(); ++i) solution_masks_[i] = compute_mask(words_.get_solutions()[i]);
}

std::shared_ptr<MemoryNode> Builder::build() {
    SolverState all_solutions(words_.get_solutions().size());
    for (size_t i = 0; i < words_.get_solutions().size(); ++i) {
        all_solutions.set(i);
    }
    return solve(all_solutions, 0);
}

// Struct to hold scoring result
struct ScoredGuess {
    int index;
    double entropy;
    int max_bucket;
};

std::shared_ptr<MemoryNode> Builder::solve(const SolverState& candidates, int depth) {
    if (candidates.count() == 0) return nullptr;

    if (cache_.count(candidates)) return cache_.at(candidates);

    int R = 6 - depth;

    // Base Case: 1 candidate
    if (candidates.count() == 1) {
        int sol_idx = candidates.get_active_indices()[0];
        auto node = std::make_shared<MemoryNode>();
        node->guess_index = solution_to_guess_[sol_idx];
        node->is_leaf = true;
        return node;
    }

    // Failure
    if (depth >= 6) return nullptr;

    // Filter relevant guesses
    std::vector<int> candidate_guesses;
    
    // Compute "active mask": union of all letters in remaining candidates
    uint32_t active_mask = 0;
    const auto& words = candidates.get_words();
    for (size_t i = 0; i < words.size(); ++i) {
        uint64_t w = words[i];
        if (w == 0) continue;
        size_t base_idx = i * 64;
        while (w) {
            int bit = __builtin_ctzll(w);
            active_mask |= solution_masks_[base_idx + bit];
            w &= (w - 1);
        }
    }
    
    if (R == 1) {
        // Solve mode: Must guess one of the remaining solutions
        const auto& indices = candidates.get_words();
        for (size_t i = 0; i < indices.size(); ++i) {
            uint64_t w = indices[i];
            size_t base_idx = i * 64;
            while (w) {
                int bit = __builtin_ctzll(w);
                candidate_guesses.push_back(solution_to_guess_[base_idx + bit]);
                w &= (w - 1);
            }
        }
    } else {
        // All relevant guesses
        candidate_guesses.reserve(words_.get_guesses().size());
        for (size_t i = 0; i < words_.get_guesses().size(); ++i) {
            // NOTE: We must ensure we don't prune the start word at root.
            if (depth == 0 || (guess_masks_[i] & active_mask) != 0) {
                candidate_guesses.push_back(i);
            }
        }
    }

    std::vector<ScoredGuess> scored_guesses;
    scored_guesses.reserve(candidate_guesses.size());

    // Force start_word at depth 0
    if (depth == 0) {
        if (!start_word_.empty()) {
            auto it = std::lower_bound(words_.get_guesses().begin(), words_.get_guesses().end(), start_word_);
            if (it != words_.get_guesses().end() && *it == start_word_) {
                int idx = std::distance(words_.get_guesses().begin(), it);
                scored_guesses.push_back({idx, 100.0, 1});
            } else {
                std::cerr << "Warning: Start word '" << start_word_ << "' not found in guesses." << std::endl;
            }
        } else {
             // Fallback logic if needed
        }
    } else {

        unsigned int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;
        
        if (candidate_guesses.size() < 100) {
             for (int g : candidate_guesses) {
                auto h = compute_heuristic(candidates, g, table_);
                if (R == 2 && h.max_bucket > 1) continue; 
                double penalty = 0.0;
                if (R == 3 && h.max_bucket > 5) penalty = 10.0;
                scored_guesses.push_back({g, h.entropy - penalty, h.max_bucket});
            }
        } else {
            std::vector<std::future<std::vector<ScoredGuess>>> futures;
            size_t chunk_size = candidate_guesses.size() / num_threads;
            
            for (unsigned int t = 0; t < num_threads; ++t) {
                size_t start = t * chunk_size;
                size_t end = (t == num_threads - 1) ? candidate_guesses.size() : start + chunk_size;
                
                futures.push_back(std::async(std::launch::async, [start, end, &candidate_guesses, &candidates, this, R]() {
                    std::vector<ScoredGuess> local_results;
                    local_results.reserve(end - start);
                    for (size_t i = start; i < end; ++i) {
                        int g = candidate_guesses[i];
                        auto h = compute_heuristic(candidates, g, table_);
                        if (R == 2 && h.max_bucket > 1) continue; 
                        
                        double penalty = 0.0;
                        if (R == 3 && h.max_bucket > 5) penalty = 10.0;
                        local_results.push_back({g, h.entropy - penalty, h.max_bucket});
                    }
                    return local_results;
                }));
            }
            
            for (auto& f : futures) {
                auto res = f.get();
                scored_guesses.insert(scored_guesses.end(), res.begin(), res.end());
            }
        }
        
        std::sort(scored_guesses.begin(), scored_guesses.end(), [](const auto& a, const auto& b) {
            return a.entropy > b.entropy;
        });
    }

    int K_values[] = {5, 50, 100000};
    
    for (int K : K_values) {
        int limit = std::min((int)scored_guesses.size(), K);
        
        for (int i = 0; i < limit; ++i) {
            int g_idx = scored_guesses[i].index;
            auto node = std::make_shared<MemoryNode>();
            node->guess_index = g_idx;
            
            bool possible = true;
            
            std::vector<std::vector<int>> bins(243);
            auto active = candidates.get_words();
            for (size_t i = 0; i < active.size(); ++i) {
                uint64_t w = active[i];
                if (w == 0) continue;
                size_t base_idx = i * 64;
                while (w) {
                    int bit = __builtin_ctzll(w);
                    size_t s = base_idx + bit;
                    bins[table_.get_pattern(g_idx, s)].push_back(s);
                    w &= (w - 1);
                }
            }
            
            for (int p = 0; p < 243; ++p) {
                if (bins[p].empty()) continue;
                
                SolverState next_state(table_.num_solutions());
                for (int s : bins[p]) next_state.set(s);
                
                if (p != 242 && next_state.count() == candidates.count()) {
                     possible = false;
                     break; 
                }

                if (p == 242) {
                    continue; 
                }
                
                auto child = solve(next_state, depth + 1);
                if (!child) {
                    possible = false;
                    break;
                }
                node->children[p] = child;
            }
            
            if (possible) {
                cache_[candidates] = node;
                return node;
            }
        }
    }
    
    return nullptr;
}

} // namespace wordle