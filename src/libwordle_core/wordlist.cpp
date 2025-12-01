#include "libwordle_core/wordlist.h"
#include <algorithm>
#include <fstream>
#include <iostream>

namespace wordle {

namespace {

uint64_t fnv1a_64(const std::vector<std::string> &words) {
  uint64_t hash = 14695981039346656037ULL;
  for (const auto &w : words) {
    for (char c : w) {
      hash ^= static_cast<uint64_t>(c);
      hash *= 1099511628211ULL;
    }
    // Add separator to ensure boundaries
    hash ^= static_cast<uint64_t>(0);
    hash *= 1099511628211ULL;
  }
  return hash;
}

bool load_file(const std::string &path, std::vector<std::string> &out) {
  std::ifstream f(path);
  if (!f.is_open()) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return false;
  }
  std::string line;
  while (std::getline(f, line)) {
    // Trim whitespace
    line.erase(line.find_last_not_of(" \n\r\t") + 1);
    if (line.empty())
      continue;
    if (line.size() != 5) {
      std::cerr << "Warning: Skipping invalid word '" << line << "' in " << path
                << std::endl;
      continue;
    }
    out.push_back(line);
  }
  std::sort(out.begin(), out.end());
  out.erase(std::unique(out.begin(), out.end()), out.end());
  return true;
}

} // namespace

bool WordList::load(const std::string &solutions_path,
                    const std::string &guesses_path) {
  solutions_.clear();
  guesses_.clear();

  if (!load_file(solutions_path, solutions_))
    return false;
  if (!load_file(guesses_path, guesses_))
    return false;

  // Checksum based on guesses (which should be the superset or at least the
  // exhaustive list used for indexing) The spec says "Checksum of sorted word
  // list". Usually this means the master list (guesses). But since solutions
  // are distinct, maybe I should combine? The binary header has ONE checksum.
  // It must verify the context is identical. I will hash the GUESSES list, as
  // that defines the indices.
  checksum_ = fnv1a_64(guesses_);

  return true;
}

} // namespace wordle
