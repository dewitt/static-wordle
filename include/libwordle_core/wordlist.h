#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace wordle {

class WordList {
public:
  WordList() = default;

  // Load solutions and guesses from files.
  // guesses_path should contain ALL valid guesses (including solutions).
  // If guesses_path is not provided or empty, it might default to solutions
  // (though usually distinct).
  bool load(const std::string &solutions_path, const std::string &guesses_path);

  const std::vector<std::string> &get_solutions() const { return solutions_; }
  const std::vector<std::string> &get_guesses() const { return guesses_; }

  // Returns a 64-bit checksum of the sorted word lists.
  uint64_t get_checksum() const { return checksum_; }

private:
  std::vector<std::string> solutions_;
  std::vector<std::string> guesses_;
  uint64_t checksum_ = 0;
};

} // namespace wordle
