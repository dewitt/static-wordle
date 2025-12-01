# TODO: Static Wordle Solver Implementation Plan

## Phase 1: Core Library & Environment
- [x] **Project Setup**
    - [x] Create directory structure (`src/`, `include/`, `tests/`, `data/`).
    - [x] Create root `CMakeLists.txt`.
    - [x] Create `src/CMakeLists.txt` for `libwordle_core`, `builder`, 
`solver`.
    - [x] Add simple testing framework.
- [x] **WordList Module** (`libwordle_core`)
    - [x] Implement `WordList` class to load `solutions.txt` and 
`guesses.txt`.
    - [x] Implement checksum (FNV-1a-64) computation for the word lists.
    - [x] *Validation*: Test loading and checksums.
- [x] **Feedback Module** (`libwordle_core`)
    - [x] Implement `calc_pattern(guess, solution)` returning 0-242.
    - [x] *Validation*: Unit tests for standard cases and edge cases (double 
letters).
- [x] **PatternTable Module** (`libwordle_core`)
    - [x] Implement `PatternTable` class.
    - [x] Implement Naive CPU generation of $P[N_g \times N_s]$.
- [x] **State Representation** (`builder`)
    - [x] Implement `SolverState` wrapper (using `std::vector<uint64_t>`) for 
solution subsets.
    - [x] Implement canonical hashing for `SolverState`.

## Phase 2: The Builder (Logic)
- [x] **Entropy & Heuristics**
    - [x] Implement Shannon entropy calculation for a given candidate set and 
guess.
    - [x] Implement "Remaining Guesses" ($R$) logic and heuristics (Max 
Entropy, Hybrid, Minimax, Solve).
    - [x] Implement Tiered Dispatcher (Scalar CPU).
- [x] **Memoization**
    - [x] Implement `GlobalCache` using `std::unordered_map`.
- [x] **Beam Search**
    - [x] Implement `solve` recursive function.
    - [x] Implement Beam Expansion Loop ($K \in \{5, 50, \text{ALL}\}$).
    - [x] Implement logic to handle valid transitions.

## Phase 3: Serialization & Verification
- [x] **Binary Writer**
    - [x] Define `DiskNode` and `Header` structs matching the spec.
    - [x] Implement `write_solution` to flatten the tree into the binary 
format.
    - [x] Write `solver_data.bin`.
- [x] **Verification**
    - [x] Implement `verify_tree` in the builder.
    - [x] Validate the tree against all 2,315 solutions.
    - [x] Ensure max depth <= 6 and valid transitions.

## Phase 4: The Solver (Runtime)
- [x] **Runtime Loader**
    - [x] Implement loader for `solver_data.bin`.
    - [x] Verify Magic and Checksum.
- [x] **Interactive CLI**
    - [x] Implement main loop:
        - Print guess.
        - Read user feedback (e.g., "GYBBG").
        - Parse feedback to pattern ID.
        - Lookup next node.
    - [x] Handle error conditions (invalid input).

## Phase 5: Optimization & Final Polish
- [x] **Performance Tuning**
    - [x] Profile memory usage (Checked during build, very efficient).
    - [x] Tune Beam Width thresholds (Standard thresholds worked).
- [x] **Documentation**
    - [x] Finalize `README.md`.
    - [x] Finalize `DESIGN.md`.