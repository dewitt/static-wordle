#include "solver.h"
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

namespace wordle {

struct Header {
  uint32_t magic;
  uint32_t version;
  uint64_t checksum;
  uint32_t num_nodes;
  uint32_t root_index;
};

bool Solver::load(const std::string &path) {
  fd_ = open(path.c_str(), O_RDONLY);
  if (fd_ == -1) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return false;
  }

  struct stat st;
  if (fstat(fd_, &st) == -1) {
    std::cerr << "Failed to get file size for: " << path << std::endl;
    close(fd_);
    fd_ = -1;
    return false;
  }
  mapped_size_ = st.st_size;

  if (mapped_size_ < sizeof(Header)) {
    std::cerr << "File too small for header: " << path << std::endl;
    close(fd_);
    fd_ = -1;
    return false;
  }

  mapped_data_ = (const uint8_t *)mmap(nullptr, mapped_size_, PROT_READ, MAP_PRIVATE, fd_, 0);
  if (mapped_data_ == MAP_FAILED) {
    std::cerr << "Failed to mmap file: " << path << std::endl;
    close(fd_);
    fd_ = -1;
    return false;
  }

  const Header *h = reinterpret_cast<const Header *>(mapped_data_);
  if (h->magic != 0x5752444C) {
    std::cerr << "Invalid Magic" << std::endl;
    return false;
  }

  checksum_ = h->checksum;
  num_nodes_ = h->num_nodes;
  root_index_ = h->root_index;

  size_t nodes_offset = sizeof(Header);
  // Check if nodes_ array fits
  if (nodes_offset + num_nodes_ * sizeof(SolverNode) > mapped_size_) {
    std::cerr << "File too small for nodes" << std::endl;
    return false;
  }
  nodes_ = reinterpret_cast<const SolverNode *>(mapped_data_ + nodes_offset);

  size_t children_offset = nodes_offset + num_nodes_ * sizeof(SolverNode);
  // Check if children_ array fits
  if (children_offset + num_nodes_ * 243 * sizeof(uint32_t) > mapped_size_) {
    std::cerr << "File too small for children" << std::endl;
    return false;
  }
  children_ =
      reinterpret_cast<const uint32_t *>(mapped_data_ + children_offset);

  // Check if root_index_ is in range
  if (root_index_ >= num_nodes_) {
    std::cerr << "Root index out of bounds" << std::endl;
    return false;
  }

  return true;
}

int Solver::get_root_index() const { return root_index_; }

const SolverNode &Solver::get_node(int index) const { return nodes_[index]; }

int Solver::get_next_node(int node_index, uint8_t pattern) const {
  return children_[node_index * 243 + pattern];
}

} // namespace wordle

wordle::Solver::~Solver() {
  if (mapped_data_ != nullptr) {
    munmap((void *)mapped_data_, mapped_size_);
    mapped_data_ = nullptr;
  }
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}
