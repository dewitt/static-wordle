#include "builder.h"
#include "entropy.h"
#include <algorithm>
#include <iostream>
#include <vector>

namespace wordle {

Builder::Builder(const WordList& words, const PatternTable& table)
    : words_(words), table_(table) {
    
    // Precompute solution -> guess mapping
    solution_to_guess_.resize(words_.get_solutions().size());
    const auto& guesses = words_.get_guesses();
    for (size_t i = 0; i < words_.get_solutions().size(); ++i) {
        const std::string& sol = words_.get_solutions()[i];
        auto it = std::lower_bound(guesses.begin(), guesses.end(), sol);
        if (it != guesses.end() && *it == sol) {
            solution_to_guess_[i] = std::distance(guesses.begin(), it);
        } else {
            // Should not happen if guesses is superset
            std::cerr << "Error: Solution " << sol << " not found in guesses!" << std::endl;
        }
    }
}

std::shared_ptr<MemoryNode> Builder::build() {
    SolverState all_solutions(words_.get_solutions().size());
    for (size_t i = 0; i < words_.get_solutions().size(); ++i) {
        all_solutions.set(i);
    }
    // Fixed start word SALET (heuristic or hardcoded).
    // The spec says "Fixed start word SALET".
    // I can enforce it by setting the first guess.
    // However, solve() picks the best. If SALET is best, great.
    // If spec requires SALET, I should start with it.
    // "Max Entropy ... Fixed start word: SALET" (Table 2.1).
    // I will let the solver run normally, but maybe for Depth 0 force "salet"?
    // The table implies "At Depth 0 (Root), use Max Entropy. Constraint: Fixed start word SALET".
    // This effectively means "Don't search, just use SALET".
    // I will implement a check in solve() or just construct the root manually.
    // Constructing manually is better to avoid recursion overhead for root.
    
    // Actually, solve() is general.
    // I will call solve() but I need to handle the "Fixed start word" requirement.
    // I'll modify solve to accept an optional "force guess".
    // Or just let it run. SALET is usually optimal.
    // But to be compliant, I should force it.
    
    // I'll just use solve() for now, and trust it picks SALET or close.
    // Actually, spec says "guarantee... starting with the fixed word SALET".
    // So I MUST use SALET at root.
    
    return solve(all_solutions, 0);
}

std::shared_ptr<MemoryNode> Builder::solve(const SolverState& candidates, int depth) {
    if (candidates.count() == 0) return nullptr;

    // Check cache
    if (cache_.count(candidates)) return cache_.at(candidates);

    int R = 6 - depth;

    // Base Case: 1 candidate
    if (candidates.count() == 1) {
        int sol_idx = candidates.get_active_indices()[0];
        auto node = std::make_shared<MemoryNode>();
        node->guess_index = solution_to_guess_[sol_idx];
        node->is_leaf = true;
        // cache_[candidates] = node; // Don't cache leaf? Cheap to recreate.
        return node;
    }

    // Failure
    if (depth >= 6) return nullptr;

    // Candidates for guessing
    std::vector<int> candidate_guesses;
    if (R == 1) {
        // Solve mode: Must guess one of the remaining solutions
        auto indices = candidates.get_active_indices();
        for (int s : indices) candidate_guesses.push_back(solution_to_guess_[s]);
    } else {
        // All guesses
        // Optimization: Use 'all_indices' precomputed?
        candidate_guesses.resize(words_.get_guesses().size());
        for (size_t i = 0; i < candidate_guesses.size(); ++i) candidate_guesses[i] = i;
    }

    // Heuristics
    struct ScoredGuess {
        int index;
        double entropy;
        int max_bucket;
    };
    std::vector<ScoredGuess> scored_guesses;
    scored_guesses.reserve(candidate_guesses.size());

    // Force "SALET" at depth 0
    if (depth == 0) {
        // Find SALET
        const std::string start_word = "salet";
        auto it = std::lower_bound(words_.get_guesses().begin(), words_.get_guesses().end(), start_word);
        if (it != words_.get_guesses().end() && *it == start_word) {
            int idx = std::distance(words_.get_guesses().begin(), it);
            scored_guesses.push_back({idx, 100.0, 1}); // Fake high score
        } else {
             std::cerr << "Warning: 'salet' not found in guesses." << std::endl;
        }
    } else {
        for (int g : candidate_guesses) {
            auto h = compute_heuristic(candidates, g, table_);
            if (R == 2 && h.max_bucket > 1) continue; // Prune
            
            // R=3 Hybrid penalty
            double penalty = 0.0;
            if (R == 3 && h.max_bucket > 5) penalty = 10.0; // Simple penalty
            
            scored_guesses.push_back({g, h.entropy - penalty, h.max_bucket});
        }
        
        // Sort
        std::sort(scored_guesses.begin(), scored_guesses.end(), [](const auto& a, const auto& b) {
            return a.entropy > b.entropy;
        });
    }

    // Beam Search
    int K_values[] = {5, 50, 100000};
    
    for (int K : K_values) {
        int limit = std::min((int)scored_guesses.size(), K);
        
        for (int i = 0; i < limit; ++i) {
            int g_idx = scored_guesses[i].index;
            auto node = std::make_shared<MemoryNode>();
            node->guess_index = g_idx;
            
            bool possible = true;
            
            // Group by pattern
            std::vector<std::vector<int>> bins(243);
            auto active = candidates.get_active_indices();
            for (int s : active) {
                bins[table_.get_pattern(g_idx, s)].push_back(s);
            }
            
            for (int p = 0; p < 243; ++p) {
                if (bins[p].empty()) continue;
                
                // Construct next state
                SolverState next_state(table_.num_solutions());
                for (int s : bins[p]) next_state.set(s);
                
                // If the next state is the same as current (no info gained), invalid move
                // unless it matches the solution (pattern 242)
                if (p != 242 && next_state.count() == candidates.count()) {
                     possible = false;
                     break; 
                }

                if (p == 242) {
                    // We found the solution!
                    // Is this a leaf node?
                    // The node we are building IS the node where we guessed the solution.
                    // So for p=242, we are done. child is null or leaf?
                    // Spec says: "Output: word_list[nodes[current_node].guess_index]"
                    // If pattern is 242, we stop.
                    // So children[242] can be null or a special marker.
                    // However, to satisfy "Leaf reached", the traversal should end.
                    // I will leave children[242] as null. The runner stops on 242.
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
