#include "builder.h"
#include "libwordle_core/patterntable.h"
#include "libwordle_core/wordlist.h"
#include "verify.h"
#include "writer.h"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::string s_path, g_path, out_path;
    std::string start_word = "reast";
    bool run_verify = false;

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
    else if (arg == "--verify")
      run_verify = true;
  }

  if (s_path.empty() || g_path.empty()) {
    std::cerr << "Usage: " << argv[0]
              << " --solutions <path> --guesses <path> [--output <path>] "
                 "[--start-word <word>] [--verify]"
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

  wordle::Builder builder(words, table, start_word);
  std::cout << "Building Tree (Start: " << start_word << ")..." << std::endl;
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
