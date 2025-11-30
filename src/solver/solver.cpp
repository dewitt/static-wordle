#include "solver.h"
#include <fstream>
#include <iostream>

namespace wordle {

struct Header {
    uint32_t magic;
    uint32_t version;
    uint64_t checksum;
    uint32_t num_nodes;
    uint32_t root_index;
};

bool Solver::load(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return false;
    
    std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);
    
    if (size < sizeof(Header)) return false;
    
    data_.resize(size);
    if (!f.read(reinterpret_cast<char*>(data_.data()), size)) return false;
    
    const Header* h = reinterpret_cast<const Header*>(data_.data());
    if (h->magic != 0x5752444C) {
        std::cerr << "Invalid Magic" << std::endl;
        return false;
    }
    
    checksum_ = h->checksum;
    num_nodes_ = h->num_nodes;
    root_index_ = h->root_index;
    
    size_t nodes_offset = sizeof(Header);
    nodes_ = reinterpret_cast<const SolverNode*>(data_.data() + nodes_offset);
    
    size_t children_offset = nodes_offset + num_nodes_ * sizeof(SolverNode);
    if (children_offset > (size_t)size) {
         std::cerr << "File too small" << std::endl;
         return false;
    }
    children_ = reinterpret_cast<const uint32_t*>(data_.data() + children_offset);
    
    return true;
}

int Solver::get_root_index() const {
    return root_index_;
}

const SolverNode& Solver::get_node(int index) const {
    return nodes_[index];
}

int Solver::get_next_node(int node_index, uint8_t pattern) const {
    return children_[node_index * 243 + pattern];
}

} // namespace wordle
