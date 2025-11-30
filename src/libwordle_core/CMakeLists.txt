add_library(wordle_core
    wordlist.cpp
    pattern.cpp
    patterntable.cpp
)
target_include_directories(wordle_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
