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
  std::vector<std::string> combined_words = guesses_;
  combined_words.insert(combined_words.end(), solutions_.begin(), solutions_.end());
  std::sort(combined_words.begin(), combined_words.end());
  checksum_ = fnv1a_64(combined_words);

  return true;
}

} // namespace wordle
