#pragma once
#include <sys/mman.h>
#include <sys/stat.h>
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

  ~Solver();

private:
  const uint8_t *mapped_data_ = nullptr;
  size_t mapped_size_ = 0;
  int fd_ = -1;
  const SolverNode *nodes_ = nullptr;
  const uint32_t *children_ = nullptr;
  int num_nodes_ = 0;
  int root_index_ = 0;
  uint64_t checksum_ = 0;
};

} // namespace wordle
