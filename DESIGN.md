# Static Wordle Solver Design Document

## 1. High-Level Architecture

The system consists of three main components:
1.  **libwordle_core**: A shared library containing domain logic (word lists, 
pattern calculation) used by both the builder and the solver.
2.  **Builder**: An offline C++ tool that constructs the optimal solution 
tree using iterative deepening beam search and serializes it to a binary file.
3.  **Solver**: A lightweight C++ CLI tool that loads the binary tree and 
plays the game in $O(1)$ time per move.

## 2. Data Structures

### 2.1 State Representation (The "Bitset")
-   **Concept**: A game state is defined exactly by the set of possible 
solutions remaining.
-   **Implementation**: A fixed-size bitset (mapping 1:1 to the sorted 
`solutions.txt`).
-   **Key**: `XXHash64` of the bitset is used as the canonical key for 
memoization.

### 2.2 Pattern Matrix ($P$)
-   **Dimensions**: $N_{guesses} 	imes N_{solutions}$ (approx. $12972 
	imes 2315$).
-   **Content**: $P[g][s]$ is the pattern ID (0-242) resulting from playing 
guess $g$ against secret $s$.
-   **Storage**: Flat array of `uint8_t`.
-   **Usage**: Used to quickly compute histograms and entropy for a set of 
candidates.

### 2.3 Solver Binary Format (`solver_data.bin`)
The artifact is a little-endian packed binary file designed for direct `mmap`.

**Header:**
-   `Magic`: "WRDL" (0x5752444C)
-   `Version`: 1
-   `ListChecksum`: XXHash64 of the sorted word list.
-   `NumNodes`: Total nodes in the tree.
-   `RootIndex`: Index of the starting node.

**Data Arrays:**
1.  **Nodes**: Array of `Node` structs.
    ```cpp
    struct Node {
        uint16_t guess_index; // Index into guesses.txt
        uint16_t flags;       // 0x1: IsLeaf, 0x2: IsSolution
    };
    ```
2.  **Children**: Array of `uint32_t`.
    -   Size: `NumNodes * 243`.
    -   Layout: Block of 243 indices for Node 0, then Node 1, etc.
    -   Lookup: `next_node = children[current_node * 243 + pattern_id]`

## 3. Core Algorithms

### 3.1 Feedback Calculation
-   Standard Wordle rules: Green (correct pos), Yellow (wrong pos), Black 
(not in word).
-   Mapped to base-3 integer: $0 is 242$.

### 3.2 Iterative Deepening Beam Search (Builder)
To ensure the solution fits within 6 guesses, the builder uses a "Remaining 
Guesses" ($R$) strategy:
-   **$R = 6 - \text{current\_depth}$**
-   **Heuristics**:
    -   $R \ge 4$: Maximize Entropy.
    -   $R = 3$: Hybrid (Penalty if max bucket size > 5).
    -   $R = 2$: Minimax (Hard constraint: Max bucket size must be 1).
        -   $R = 1$: Solve (Must guess the single remaining word).
    
    **Configurable Start Word:**
    The builder supports a `--start-word` argument. Experiments using `scripts/find_optimal_opener.py` found that **`reast`** yields a lower average guess count (3.602) compared to the canonical `salet` (3.612). Therefore, **`reast`** is the default start word.
    
    **Beam Search:**
-   For a given state, generate heuristics for all valid guesses.
-   Try beam widths $K \in \{5, 50, \text{ALL}\}$.
-   Process top $K$ candidates in parallel.
-   First valid subtree found sets an atomic success flag, cancelling other 
threads.
-   **Memoization**: A global concurrent hash map stores results of visited 
states to handle transpositions.

### 3.3 Verification
-   The builder includes a mandatory `--verify` step.
-   It iterates through all 2,315 solutions, simulating the game using the 
generated tree.
-   Asserts correctness, max depth $\le 6$, and valid transitions.

## 4. Hardware Acceleration & Optimizations
-   **Tier 1 (GPU)**: Not implemented (No CUDA hardware available).
-   **Tier 2 (CPU Optimized)**: 
    -   **Parallel Pattern Table**: `PatternTable` generation is parallelized 
across guesses using `std::async`.
    -   **Integer-Based Pattern Calc**: Replaced string-based `calc_pattern` 
with a specialized integer-based version working on pre-packed `uint8_t[5]` 
arrays to minimize overhead.
    -   **Parallel Beam Search**: `std::async` is used to parallelize entropy 
calculations across all available cores.
    -   **Efficient Bitset Iteration**: Replaced `std::vector` allocation 
with direct word-level iteration and `__builtin_ctzll` (Count Trailing Zeros).
    -   **Entropy Lookup Table**: Replaced expensive `std::log2` calls with a 
precomputed lookup table for $x \log_2 x$.
    -   **Active Character Pruning**: Implemented filtering to skip guesses 
sharing no letters with active candidates (marginal impact on small datasets).
    -   **Result**: 
        -   Pattern Table Generation: ~2.5s $\to$ ~0.3s.
        -   Tree Build Time: ~60s $\to$ ~1.0s.
        -   **Total**: ~1.3s on ARM64.
-   **Tier 3 (Scalar)**: Fallback logic (superseded by Tier 2).

## 5. Modules Responsibilities

-   **libwordle_core**: Data loading, checksums, core math ($P$ matrix, 
patterns).
-   **builder**: Search logic, threading, memoization, heuristic evaluation, 
binary writer.
-   **solver**: Binary loader, user interaction, state tracking.
