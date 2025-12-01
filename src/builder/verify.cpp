#include "verify.h"
#include "libwordle_core/pattern.h"
#include <iostream>

namespace wordle {

bool verify_tree(std::shared_ptr<MemoryNode> root, const WordList &words) {
  int max_depth = 0;
  size_t total_guesses = 0;
  bool all_valid = true;

  const auto &solutions = words.get_solutions();
  const auto &guesses = words.get_guesses();

  std::cout << "Verifying tree against " << solutions.size() << " solutions..."
            << std::endl;

  for (size_t s_idx = 0; s_idx < solutions.size(); ++s_idx) {
    const std::string &secret = solutions[s_idx];

    auto node = root;
    int depth = 0;
    bool found = false;

    while (node) {
      depth++;
      const std::string &guess = guesses[node->guess_index];
      uint8_t p = calc_pattern(guess, secret);

      if (p == 242) { // GGGGG
        // Game over.
        found = true;
        break;
      }

      if (depth >= 6) {
        std::cerr << "Fail: Depth limit exceeded for " << secret
                  << " (Last guess: " << guess << ")" << std::endl;
        all_valid = false;
        break;
      }

      // Transition
      if (p >= node->children.size() || !node->children[p]) {
        std::cerr << "Fail: Invalid transition for " << secret << " at guess "
                  << guess << " pattern " << (int)p << std::endl;
        all_valid = false;
        break;
      }
      node = node->children[p];
    }

    if (!found && all_valid) {
      std::cerr << "Fail: Did not find " << secret << std::endl;
      all_valid = false;
    }
    if (depth > max_depth)
      max_depth = depth;
    total_guesses += depth;
  }

  if (all_valid) {
    double average = static_cast<double>(total_guesses) / solutions.size();
    std::cout << "Verification Passed! Max Depth: " << max_depth << std::endl;
    std::cout << "Average Guesses: " << average << std::endl;
  } else {
    std::cout << "Verification FAILED." << std::endl;
  }
  return all_valid;
}

} // namespace wordle
