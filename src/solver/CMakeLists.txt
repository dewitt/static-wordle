add_executable(wordle_solver
    main.cpp
    solver.cpp
)
target_link_libraries(wordle_solver PRIVATE wordle_core)
