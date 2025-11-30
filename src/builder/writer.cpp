#include "writer.h"
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <iostream>

namespace wordle {

struct Header {
    uint32_t magic = 0x5752444C;
    uint32_t version = 1;
    uint64_t checksum = 0;
    uint32_t num_nodes = 0;
    uint32_t root_index = 0;
};

struct DiskNode {
    uint16_t guess_index;
    uint16_t flags; 
};

bool write_solution(const std::string& path, std::shared_ptr<MemoryNode> root, const WordList& words) {
    if (!root) return false;

    // Flatten tree
    std::vector<std::shared_ptr<MemoryNode>> flat_nodes;
    std::unordered_map<std::shared_ptr<MemoryNode>, uint32_t> node_map;
    
    // BFS
    flat_nodes.push_back(root);
    node_map[root] = 0;
    
    size_t head = 0;
    while(head < flat_nodes.size()) {
        auto node = flat_nodes[head++];
        
        for (auto child : node->children) {
            if (child && node_map.find(child) == node_map.end()) {
                node_map[child] = (uint32_t)flat_nodes.size();
                flat_nodes.push_back(child);
            }
        }
    }
    
    std::cout << "Writing " << flat_nodes.size() << " nodes to " << path << std::endl;
    
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    
    Header header;
    header.checksum = words.get_checksum();
    header.num_nodes = (uint32_t)flat_nodes.size();
    header.root_index = 0;
    
    out.write(reinterpret_cast<const char*>(&header), sizeof(Header));
    
    // Write Nodes
    for (const auto& node : flat_nodes) {
        DiskNode dn;
        dn.guess_index = node->guess_index;
        dn.flags = 0;
        if (node->is_leaf) dn.flags |= 1; // IsLeaf
        if (node->is_leaf) dn.flags |= 2; // IsSolution (Implicitly yes for leaf)
        
        out.write(reinterpret_cast<const char*>(&dn), sizeof(DiskNode));
    }
    
    // Write Children
    for (const auto& node : flat_nodes) {
        std::vector<uint32_t> children_indices(243, 0xFFFFFFFF);
        
        for (int i = 0; i < 243; ++i) {
            if (node->children[i]) {
                children_indices[i] = node_map[node->children[i]];
            }
        }
        out.write(reinterpret_cast<const char*>(children_indices.data()), 243 * sizeof(uint32_t));
    }
    
    out.close();
    return true;
}

} // namespace wordle
