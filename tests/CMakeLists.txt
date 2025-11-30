add_executable(test_core test_core.cpp)
target_link_libraries(test_core PRIVATE wordle_core)
add_test(NAME CoreTest COMMAND test_core)
