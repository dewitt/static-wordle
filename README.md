# Static Constant-Time Wordle Solver

A high-performance, static Wordle solver that guarantees a win in $\le 6$ guesses for every standard Wordle solution. It precomputes a complete solution tree and compiles it into a compact binary artifact for $O(1)$ runtime lookups.

## Features
-   **Guaranteed Win**: Mathematically proven strategy for all 2,315 solutions.
-   **Constant Time**: Runtime solver performs simple array lookups.
-   **High-Performance Builder**: Uses parallel iterative deepening beam search, AVX/SIMD optimizations, and optional CUDA acceleration.
-   **Verifiable**: Automatically verifies the solution tree against all 2,315 solutions before generating the binary.

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
The builder will automatically verify the tree against all solutions before writing the output.

```bash
# Generate solver_data.bin with default start word (salet)
./bin/wordle_builder --solutions ../data/solutions.txt --guesses ../data/guesses.txt --output solver_data.bin

# Generate with a custom start word (e.g., reast)
./bin/wordle_builder --solutions ../data/solutions.txt --guesses ../data/guesses.txt --output solver_data.bin --start-word reast
```

### 2. Finding Optimal Openers
A Python script is provided to test various starting words to minimize the average guess count.

```bash
python3 ../scripts/find_optimal_opener.py
```

### 3. Running the Solver
The solver uses the generated binary to play interactively.

```bash
./bin/wordle_solver solver_data.bin ../data/solutions.txt ../data/guesses.txt
```

**Interaction:**
-   The solver suggests a word (defaults to **salet** or your chosen start word).
-   Input the feedback from the game using characters:
    -   `G`: Green
    -   `Y`: Yellow
    -   `B`: Black
    -   Example: `GYBBG`

### 3. Non-Interactive Mode
To see the solution path for a specific word automatically:

```bash
./bin/wordle_solver solver_data.bin --solve react ../data/solutions.txt ../data/guesses.txt
```

Output:
```
Solving for target: react
Guess 1: salet (BYBYG)
Guess 2: crena (YYYBY)
Guess 3: react (GGGGG)
Solved in 3 guesses! (6 µs)
```

## Architecture
See [DESIGN.md](DESIGN.md) for architectural details.