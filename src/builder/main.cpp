#include "builder.h"
#include "libwordle_core/patterntable.h"
#include "libwordle_core/wordlist.h"
#include "verify.h"
#include "writer.h"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char **argv) {
  std::string s_path, g_path, out_path;
  // Default to "trace" (avg 3.605) instead of "reast" (avg 3.602) because
  // "trace" is a valid solution, offering a chance for a 1-guess win.
  std::string start_word = "trace";
  bool run_verify = false;
  std::string single_list_path;
  wordle::HeuristicType heuristic = wordle::HeuristicType::ENTROPY;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--solutions" && i + 1 < argc)
      s_path = argv[++i];
    else if (arg == "--guesses" && i + 1 < argc)
      g_path = argv[++i];
    else if (arg == "--output" && i + 1 < argc)
      out_path = argv[++i];
    else if (arg == "--start-word" && i + 1 < argc)
      start_word = argv[++i];
    else if (arg == "--single-list" && i + 1 < argc)
      single_list_path = argv[++i];
    else if (arg == "--heuristic" && i + 1 < argc) {
      std::string h = argv[++i];
      if (h == "min_expected")
        heuristic = wordle::HeuristicType::MIN_EXPECTED;
      else if (h == "entropy")
        heuristic = wordle::HeuristicType::ENTROPY;
      else {
        std::cerr << "Unknown heuristic: " << h
                  << " (use 'entropy' or 'min_expected')" << std::endl;
        return 1;
      }
    } else if (arg == "--verify")
      run_verify = true;
  }

  if (!single_list_path.empty()) {
    s_path = single_list_path;
    g_path = single_list_path;
  }

  if (s_path.empty() || g_path.empty()) {
    std::cerr << "Usage: " << argv[0]
              << " (--solutions <path> --guesses <path> | --single-list "
                 "<path>) [--output <path>] [--start-word <word>] [--heuristic "
                 "entropy|min_expected] [--verify]"
              << std::endl;
    return 1;
  }

  wordle::WordList words;
  if (!words.load(s_path, g_path))
    return 1;

  std::cout << "Loaded " << words.get_solutions().size() << " solutions."
            << std::endl;
  std::cout << "Loaded " << words.get_guesses().size() << " guesses."
            << std::endl;

  wordle::PatternTable table;
  std::cout << "Generating Pattern Table..." << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  table.generate(words.get_guesses(), words.get_solutions());
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Table generated in "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << "ms" << std::endl;

  wordle::Builder builder(words, table, start_word, heuristic);
  std::cout << "Building Tree (Start: " << start_word << ", Heuristic: "
            << (heuristic == wordle::HeuristicType::ENTROPY ? "Entropy"
                                                            : "MinExpected")
            << ")..." << std::endl;
  start = std::chrono::high_resolution_clock::now();
  auto root = builder.build();
  end = std::chrono::high_resolution_clock::now();
  std::cout << "Build time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << "ms" << std::endl;

  if (!root) {
    std::cout << "Failed to build tree." << std::endl;
    return 1;
  }

  std::cout << "Success! Root Guess Index: " << root->guess_index << " ("
            << words.get_guesses()[root->guess_index] << ")" << std::endl;

  // Verify
  if (!wordle::verify_tree(root, words)) {
    std::cerr << "Tree verification failed! Aborting write." << std::endl;
    return 1;
  }

  // Write
  if (!out_path.empty()) {
    std::cout << "Writing to " << out_path << "..." << std::endl;
    if (wordle::write_solution(out_path, root, words)) {
      std::cout << "Successfully wrote " << out_path << std::endl;
    } else {
      std::cerr << "Failed to write " << out_path << std::endl;
      return 1;
    }
  }

  return 0;
}
