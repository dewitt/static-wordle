#include "state.h"
#include <algorithm>
#include <cstring>

namespace wordle {

SolverState::SolverState(size_t num_solutions) : size_(num_solutions) {
    size_t num_words = (num_solutions + 63) / 64;
    bits_.resize(num_words, 0);
}

void SolverState::set(size_t index) {
    if (index < size_) {
        bits_[index / 64] |= (1ULL << (index % 64));
    }
}

bool SolverState::get(size_t index) const {
    if (index >= size_) return false;
    return (bits_[index / 64] >> (index % 64)) & 1ULL;
}

size_t SolverState::count() const {
    size_t c = 0;
    for (auto w : bits_) {
        c += __builtin_popcountll(w);
    }
    return c;
}

bool SolverState::empty() const {
    for (auto w : bits_) {
        if (w != 0) return false;
    }
    return true;
}

std::vector<int> SolverState::get_active_indices() const {
    std::vector<int> indices;
    indices.reserve(count());
    for (size_t i = 0; i < size_; ++i) {
        if (get(i)) indices.push_back(static_cast<int>(i));
    }
    return indices;
}

uint64_t SolverState::hash() const {
    // Simple FNV-1a like mix for now, or something better.
    // Using a simple mix.
    uint64_t h = 14695981039346656037ULL;
    for (auto w : bits_) {
        h ^= w;
        h *= 1099511628211ULL;
    }
    return h;
}

bool SolverState::operator==(const SolverState& other) const {
    if (size_ != other.size_) return false;
    return bits_ == other.bits_;
}

} // namespace wordle
