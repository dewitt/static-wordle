#pragma once
#include "builder.h"
#include <string>

namespace wordle {

bool write_solution(const std::string& path, std::shared_ptr<MemoryNode> root, const WordList& words);

}
