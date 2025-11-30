#pragma once
#include "state.h"
#include "libwordle_core/wordlist.h"
#include "libwordle_core/patterntable.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace wordle {

struct MemoryNode {
    uint16_t guess_index = 0;
    bool is_leaf = false;
    // Children: Index by pattern (0-242).
    // If nullptr, that pattern is impossible.
    std::vector<std::shared_ptr<MemoryNode>> children; 
    
    MemoryNode() : children(243, nullptr) {}
};

class Builder {
public:
    Builder(const WordList& words, const PatternTable& table);
    
    std::shared_ptr<MemoryNode> build();

private:
    std::shared_ptr<MemoryNode> solve(const SolverState& candidates, int depth);

    const WordList& words_;
    const PatternTable& table_;
    
    std::unordered_map<SolverState, std::shared_ptr<MemoryNode>> cache_;
    std::vector<int> solution_to_guess_;

    // Helper to get next state given a guess and pattern
    SolverState filter_candidates(const SolverState& current, int guess_idx, uint8_t pattern);
};

} // namespace wordle
