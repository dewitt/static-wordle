add_executable(wordle_builder
    main.cpp
    state.cpp
    entropy.cpp
    builder.cpp
    writer.cpp
    verify.cpp
)
target_link_libraries(wordle_builder PRIVATE wordle_core)
