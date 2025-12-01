#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace wordle {

struct SolverNode {
  uint16_t guess_index;
  uint16_t flags;
};

class Solver {
public:
  bool load(const std::string &path);

  int get_root_index() const;
  const SolverNode &get_node(int index) const;
  int get_next_node(int node_index, uint8_t pattern) const;

  uint64_t get_checksum() const { return checksum_; }

private:
  std::vector<uint8_t> data_;
  const SolverNode *nodes_ = nullptr;
  const uint32_t *children_ = nullptr;
  int num_nodes_ = 0;
  int root_index_ = 0;
  uint64_t checksum_ = 0;
};

} // namespace wordle
