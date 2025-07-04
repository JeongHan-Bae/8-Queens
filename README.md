# ♛ 8-Queens Bitboard Solver

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Header-only](https://img.shields.io/badge/header--only-✓-brightgreen)]()
[![No Dependencies](https://img.shields.io/badge/dependencies-none-success)]()
[![License: MIT](https://img.shields.io/badge/license-MIT-yellow.svg)]()

A **pure header-only**, **zero-dependency**, **bitboard-based** solver for the classical **8-Queens Problem**, written in **modern C++20**.

> ⚠️ **Not a library**. This is a performance-focused, branch-aware, constexpr-rich implementation — written for fun, learning, and algorithmic elegance.  
> It's minimal, fast, and over-engineered — and that's the point.

---

## ✨ Features

- ♟ **Bitboard representation** — the entire 8×8 board is a single `uint64_t`
- 🚀 **Stack-based DFS** — non-recursive, allocation-free backtracking
- 🧮 **Precomputed attack masks** — fast mask updates via `consteval`
- 🔁 **Symmetry-aware deduplication** — canonicalizes solutions under all 8 transforms
- 📦 **Header-only & dependency-free** — just drop in and go
- 🧼 **Clean namespace & POD-based internals** — trivial to inspect or extend

---

## ⚙️ Implementation Details

- **True value of the `kill_table`**  
  It’s not just about “pushing computations to compile time.” At runtime, all branch decisions (about 2,057 branches measured, minus 92 base cases) are reduced to a single table lookup, while only 64 bitwise operations are actually performed.

- **Role of `constexpr rotate*` / `flip*` / `canonical`**  
  These are primarily semantic hints that these functions are pure and side-effect-free, not mandates to perform the work at compile time. Marking them `constexpr` enables LLVM at the IR level to “see through” loops, conditionals, and bitmask operations—allowing aggressive constant propagation, dead-code elimination, loop unrolling, and even replacing handwritten loops with specialized bit-manipulation instructions (e.g., BMI2) for optimal assembly.

-  **`constexpr`-enabled Link Time Optimization**  
  Since `constexpr` implies both `inline` and `noexcept` for eligible functions, it guarantees no side effects and allows Link Time Optimization to merge, deduplicate, and eliminate any remaining overhead across module boundaries.
- **Performance on Darwin M3 (`-O3 -march=native`)**  
  Full enumeration of all 92 solutions takes an average of **26–30 μs**.  
  Full enumeration of all 12 unique solutions takes an average of **33–48 μs**.  

---

## 🛠 Requirements

- A **C++20**-compliant compiler (tested with GCC 12+, Clang 14+)
- No external dependencies
- No build system required — it's just a single header

---

## 📦 Usage

```cpp
#include "Queens.hpp"
#include <iostream>

int main() {
    auto all = queens::queens_problem();          // All 92 valid solutions
    auto unique = queens::queens_problem_uniq();  // 12 unique (under symmetry)

    std::cout << "Total solutions: " << all.size() << "\n";
    std::cout << "Unique solutions: " << unique.size() << "\n";

    std::cout << queens::to_string(*unique.begin()); // Render one board
}
````

Each solution is a `uint64_t` where each bit represents one square:

* Bit 0 = (row 0, col 0), Bit 63 = (row 7, col 7)
* Use `queens::to_string()` to view the board

---

## 📷 Example Output

```plaintext
. . . Q . . . . 
. . . . . . Q . 
Q . . . . . . . 
. . . . . . . Q 
. . . . Q . . . 
. Q . . . . . . 
. . . . . Q . . 
. . Q . . . . . 
```

---

## 🖨️ ASCII Rendering

The helper function below converts a `grid` to a printable string:

```cpp
std::string queens::to_string(grid g);
```

Usage:

```cpp
std::cout << queens::to_string(some_solution);
```

---

## 📂 File Layout

```
Queens.hpp       # The entire solver (single header)
```

No build. No link step. No install. Just include and run.

---

## 🔍 Internals at a Glance

* `grid` = `uint64_t`, 1 bit per square
* Manual `std::stack<iter>` replaces recursion
* `kill_table` precomputes queen attack masks
* 8 symmetries via `rotate`, `flip` utilities
* `canonical()` gives lex-min form for deduplication

---

## 💡 Why?

Because it's fun to write something:

* that fits in your L1 cache,
* runs in milliseconds,
* and still looks elegant in C++.

It's a minimal and over-optimized solution to a toy problem — and that's the charm.

---

## ⚖ License

MIT License © 2025 [JeongHan Bae](https://github.com/JeongHan-Bae)
