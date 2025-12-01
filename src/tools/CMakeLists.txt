add_executable(rank_openers rank_openers.cpp)

# We need objects from builder (entropy, state) but not the whole builder app.
# A cleaner way would be to make a static library 'wordle_solver_lib', 
# but for now we link the objects directly.
target_sources(rank_openers PRIVATE 
    ${CMAKE_SOURCE_DIR}/src/builder/entropy.cpp 
    ${CMAKE_SOURCE_DIR}/src/builder/state.cpp
)

target_include_directories(rank_openers PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(rank_openers PRIVATE wordle_core)
