# Static Constant-Time Wordle Solver

A high-performance, static Wordle solver that guarantees a win in $\le 6$ 
guesses for every standard Wordle solution. It precomputes a complete 
solution tree and compiles it into a compact binary artifact for $O(1)$ 
runtime lookups.

## Features
-   **Guaranteed Win**: Mathematically proven strategy for all 2,315 
    solutions (Max depth 5 with default settings).
-   **Constant Time**: Runtime solver performs simple array lookups.
-   **High-Performance Builder**: Optimized C++17 implementation using 
    multithreading, integer-based pattern calculation, and efficient bitset 
    manipulation. Builds the full tree in ~1.1 seconds on standard hardware.
-   **Verifiable**: Automatically verifies the solution tree against all 
    2,315 solutions before generating the binary.
-   **Configurable Strategy**: Supports custom start words and pluggable 
    heuristics.

## Building

### Requirements
-   C++17 compiler (GCC, Clang, or MSVC)
-   CMake 3.15+

### Instructions
```bash
mkdir build && cd build
cmake ..
make -j
```

## Usage

### 1. Generating the Solver Data
The builder requires `solutions.txt` and `guesses.txt` (standard Wordle lists). 
It generates a binary file (`solver_data.bin`) used by the runtime solver.

**Standard Build (Default start word: `trace`):**
```bash
./bin/wordle_builder --solutions ../data/solutions.txt --guesses ../data/guesses.txt --output solver_data.bin
```

**Strategy Note:**
The default start word is **`trace`** (Average guesses: 3.606). While **`reast`** offers a slightly lower average (3.602), it is not a valid solution word. `trace` is chosen because it allows for a lucky 1-guess victory.

**Custom Start Word:**
```bash
./bin/wordle_builder --solutions ../data/solutions.txt --guesses ../data/guesses.txt --output solver_data.bin --start-word reast
```

**Hard Mode (Single list for solutions and guesses):**
```bash
./bin/wordle_builder --single-list ../data/guesses.txt --output solver_data.bin
```

**Advanced Options:**
-   `--heuristic <type>`: Choose the splitting strategy. Options: `entropy` (default), `min_expected`.

### 2. Analysis Tools

**Rank Openers by Entropy:**
Quickly calculate the Shannon entropy for all words to identify strong start word candidates.
```bash
./bin/rank_openers ../data/solutions.txt ../data/guesses.txt 100 > top_100.txt
```

**Find Optimal Opener:**
A Python script that runs the builder against a list of words to find the one with the lowest average guess count.
```bash
# Test default list
python3 ../scripts/find_optimal_opener.py

# Test words from a file (e.g., output of rank_openers)
python3 ../scripts/find_optimal_opener.py top_100.txt
```

### 3. Running the Solver
The solver uses the generated binary to play interactively.

```bash
./bin/wordle_solver solver_data.bin ../data/solutions.txt ../data/guesses.txt
```

**Interaction:**
-   The solver suggests a word (defaults to **trace** or your chosen start word).
-   Input the feedback from the game using characters:
    -   `G`: Green
    -   `Y`: Yellow
    -   `B`: Black
    -   Example: `GYBBG`

### 4. Non-Interactive Mode (Simulation)
To automatically simulate the game for a specific solution word:

```bash
./bin/wordle_solver solver_data.bin --solve react ../data/solutions.txt ../data/guesses.txt
```

**Output:**
```
Solving for target: react
Guess 1: reast (YYBBY)
Guess 2: crate (BGBBG)
Guess 3: tract (BGBGG)
Guess 4: react (GGGGG)
Solved in 4 guesses! (12 µs)
```

### 5. Benchmarking
To run the solver against all 2,315 solutions and measure aggregate performance:

```bash
./bin/wordle_solver solver_data.bin ../data/solutions.txt ../data/guesses.txt --benchmark
```

**Output:**
```
Benchmarking against all 2315 solutions...
Solved 2315 games in 0.67 ms.
Average time per game: 0.29 µs.
Average guesses: 3.60216
```

## Architecture
See [DESIGN.md](DESIGN.md) for detailed architectural documentation and optimization findings.
