#pragma once
#include <cstdint>
#include <functional>
#include <vector>

namespace wordle {

class SolverState {
public:
  SolverState() = default;
  SolverState(size_t num_solutions);

  void set(size_t index);
  bool get(size_t index) const;
  size_t count() const;
  bool empty() const;

  // Returns indices of set bits
  std::vector<int> get_active_indices() const;

  const std::vector<uint64_t> &get_words() const { return bits_; }

  uint64_t hash() const;

  bool operator==(const SolverState &other) const;
  bool operator!=(const SolverState &other) const { return !(*this == other); }

private:
  std::vector<uint64_t> bits_;
  size_t size_ = 0;
};

} // namespace wordle

namespace std {
template <> struct hash<wordle::SolverState> {
  size_t operator()(const wordle::SolverState &s) const { return s.hash(); }
};
} // namespace std
