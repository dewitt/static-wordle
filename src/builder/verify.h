#pragma once
#include "builder.h"
#include "libwordle_core/wordlist.h"

namespace wordle {

bool verify_tree(std::shared_ptr<MemoryNode> root, const WordList& words);

}
