#!/usr/bin/env python3
import subprocess
import re
import os

WORDS_TO_TEST = [
    "salet", "trace", "crate", "slate", "reast", "soare", "roate", "raise"
]

BUILDER_BIN = "./build/bin/wordle_builder"
SOLUTIONS = "data/solutions.txt"
GUESSES = "data/guesses.txt"
OUTPUT_BIN = "solver_data.bin"

results = []

print(f"Testing {len(WORDS_TO_TEST)} starting words...")

for word in WORDS_TO_TEST:
    print(f"Testing '{word}'...", end="", flush=True)
    
    cmd = [
        BUILDER_BIN,
        "--solutions", SOLUTIONS,
        "--guesses", GUESSES,
        "--output", OUTPUT_BIN,
        "--start-word", word
    ]
    
    try:
        # Run builder and capture output
        proc = subprocess.run(cmd, capture_output=True, text=True)
        
        if proc.returncode != 0:
            print(" Failed.")
            print(proc.stderr)
            continue
            
        # Parse "Average Guesses: 3.xyz"
        match = re.search(r"Average Guesses: (\d+\.\d+)", proc.stdout)
        if match:
            avg = float(match.group(1))
            results.append((word, avg))
            print(f" Average: {avg}")
        else:
            print(" Could not parse average.")
            
    except Exception as e:
        print(f" Error: {e}")

print("\n--- Results ---")
results.sort(key=lambda x: x[1])

for word, avg in results:
    print(f"{word}: {avg}")

print(f"\nBest Opener: {results[0][0]} ({results[0][1]})")
