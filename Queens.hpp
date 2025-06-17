/**
 * @file Queens.hpp [C++20]
 * @brief A compact and efficient bitboard-based solution for the 8-Queens problem.
 *
 * This header-only implementation provides enumeration of all possible
 * 8-Queens board configurations, including symmetry-aware unique solutions.
 *
 * Bit-level operations are used to represent and manipulate the chessboard efficiently.
 *
 * @author JeongHan Bae
 * @version 1.0
 * @date 2025
 */

#pragma once

#include <cstdint>
#include <stack>
#include <vector>
#include <unordered_set>

namespace queens {

    /**
     * @brief Represents an 8x8 chessboard using 64-bit unsigned integer.
     *
     * Each bit corresponds to a cell on the board:
     * - Bit 0 = cell (0,0), Bit 1 = (0,1), ..., Bit 63 = (7,7).
     * A bit value of 1 indicates the cell is available or occupied.
     */
    using grid = std::uint64_t;

    /**
     * @brief Initial state of the board with all cells available.
     */
    constexpr grid init_grid = ~(0ULL);

    namespace detail {

        /**
         * @brief Represents a lookup table of grid masks for each queen attack position.
         *
         * Each entry gives the set of positions that would be killed (attacked)
         * by placing a queen at the corresponding cell.
         */
        struct array {
            std::uint64_t data[64];

            std::uint64_t operator[](std::uint8_t pos) const {
                return data[pos];
            }

            /**
             * @brief Get the bitmask for cells attacked by a queen at (row, col).
             */
            [[nodiscard]] std::uint64_t pos(std::uint8_t row, std::uint8_t col) const {
                return data[row * 8 + col];
            }
        };

        /**
         * @brief Lightweight structure for stack-based DFS state.
         *
         * This struct is used purely to pack the search state for each level
         * of the recursive queen placement during iteration.
         *
         * It is a Plain Old Data (POD) type intentionally:
         * - No constructors, destructors, or member functions.
         * - Allows for trivial initialization, copying, and fast allocation.
         * - Enables optimal use in std::stack with minimal overhead.
         *
         * This design supports performance-critical backtracking without requiring any runtime behavior.
         */
        struct iter {
            [[maybe_unused]] grid queen_grid; ///< Current board availability
            [[maybe_unused]] std::uint8_t row; ///< Current row to place a queen
            // [[maybe_unused]] used to suppress warnings of old clang-tidy
        };
    }

    /**
     * @brief Gets the bitmask for a column.
     * @param col Column index [0,7]
     * @return Bitmask with 1s in the specified column
     */
    constexpr grid get_col(std::uint8_t col) {
        return 0x0101010101010101ULL << col;
    }

    /**
     * @brief Gets the bitmask for a row.
     * @param row Row index [0,7]
     * @return Bitmask with 1s in the specified row
     */
    constexpr grid get_row(std::uint8_t row) {
        return 0xFFULL << (row * 8);
    }

    /**
     * @brief Gets the bitmask for diagonals (main + anti) that pass through a given cell.
     * @param row Row index
     * @param col Column index
     * @return Bitmask covering both diagonals intersecting at (row, col)
     */
    constexpr grid get_diag(std::uint8_t row, std::uint8_t col) {
        constexpr grid main_diag = 0x0102040810204080ULL;
        constexpr grid anti_diag = 0x8040201008040201ULL;

        grid diag = (row + col > 7) ?
                    main_diag << ((row + col - 7) * 8)
                                    : main_diag >> ((7 - row - col) * 8);
        grid rev_diag = (row > col) ?
                        anti_diag << ((row - col) * 8)
                                    : anti_diag >> ((col - row) * 8);

        return diag | rev_diag;
    }

    namespace detail {
        /**
         * @brief Reverse bits in a byte.
         */
        constexpr std::uint8_t reverse_byte(std::uint8_t b) {
            b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
            b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
            b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
            return b;
        }

        /**
         * @brief Horizontal flip of the grid (mirror along vertical axis).
         */
        constexpr grid flip_horizontal(grid g) {
            grid result = 0;
            for (int row = 0; row < 8; ++row) {
                std::uint8_t line = (g >> (row * 8)) & 0xFF;
                result |= static_cast<grid>(reverse_byte(line)) << (row * 8);
            }
            return result;
        }

        /**
         * @brief Vertical flip of the grid (mirror along horizontal axis).
         */
        constexpr grid flip_vertical(grid g) {
            grid result = 0;
            for (int row = 0; row < 8; ++row) {
                grid line = (g >> (row * 8)) & 0xFF;
                result |= line << ((7 - row) * 8);
            }
            return result;
        }

        /**
         * @brief Flip grid along the main diagonal (top-left to bottom-right).
         */
        constexpr grid flip_diag_main(grid g) {
            grid result = 0;
            for (int r = 0; r < 8; ++r)
                for (int c = 0; c < 8; ++c)
                    if ((g >> (r * 8 + c)) & 1ULL)
                        result |= 1ULL << (c * 8 + r);
            return result;
        }

        /**
         * @brief Rotate the grid 90 degrees clockwise.
         */
        constexpr grid rotate90(grid g) {
            return flip_vertical(flip_diag_main(g));
        }

        /**
         * @brief Rotate the grid 180 degrees.
         */
        constexpr grid rotate180(grid g) {
            return flip_vertical(flip_horizontal(g));
        }

        /**
         * @brief Rotate the grid 270 degrees clockwise.
         */
        constexpr grid rotate270(grid g) {
            return flip_horizontal(flip_diag_main(g));
        }

    }

    /**
     * @brief Returns the canonical form of a board layout under all 8 symmetry transformations.
     *
     * Used for detecting unique solutions by ignoring symmetric equivalents.
     *
     * @param g Original grid
     * @return The smallest grid under all 8 symmetries
     */
    constexpr grid canonical(grid g) {
        grid forms[8] = {
                g,
                detail::rotate90(g),
                detail::rotate180(g),
                detail::rotate270(g),
                detail::flip_horizontal(g),
                detail::rotate90(detail::flip_horizontal(g)),
                detail::rotate180(detail::flip_horizontal(g)),
                detail::rotate270(detail::flip_horizontal(g))
        };
        return *std::min_element(std::begin(forms), std::end(forms));
    }

    namespace detail {
        /**
         * @brief Generates a precomputed table of attacked positions for all cells.
         *
         * For each cell, calculates the set of cells that would be killed (attacked)
         * by a queen placed there.
         *
         * @return The array of attack masks for all 64 cells
         */
        consteval array generate_kill_table() {
            array result{};

            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    grid self = 1ULL << (row * 8 + col);
                    grid row_mask = get_row(row);
                    grid col_mask = get_col(col);
                    grid diag_mask = get_diag(row, col);
                    result.data[row * 8 + col] = (row_mask | col_mask | diag_mask) & ~self;
                }
            }

            return result;
        }

        constexpr std::uint8_t zero_to_seven[8]{0, 1, 2, 3, 4, 5, 6, 7};

        /**
         * @brief Precomputed lookup table for queen attack masks.
         */
        constexpr array kill_table = generate_kill_table();

        /**
        * @brief DFS stack-based backtracking solver.
        *
        * Pushes valid queen positions into the stack while maintaining attack masks.
        *
        * @param queen_stack Stack of board states
        * @param results Output vector to store valid complete boards
        */
        void queens_helper(std::stack<iter, std::vector<iter>> &queen_stack, std::vector<grid> &results) {
            while (!queen_stack.empty()) {
                const auto [queen_grid, row] = queen_stack.top();
                queen_stack.pop();
                if (row == 8) [[unlikely]] {
                    // theoretically only 92 results, very unlikely
                    results.emplace_back(queen_grid);
                    continue;
                }
                const auto candidates = queen_grid >> (row * 8) & 0xFFULL;

                for (const auto col: zero_to_seven) {
                    if (candidates & 1 << col) {
                        // Do NOT mark with likely/unlikely — pruning patterns vary too much across branches
                        queen_stack.emplace(queen_grid & ~(kill_table.pos(row, col)), row + 1);
                        // perfect forwarding
                    }
                }
            }
        }
    }

    /**
     * @brief Solves the 8-Queens problem and returns all 92 valid board configurations.
     *
     * Uses iterative DFS with precomputed masks for efficiency.
     *
     * @return Vector of all valid 8-Queens solutions
     */
    [[maybe_unused]] std::vector<grid> queens_problem() {
        std::vector<grid> res;
        res.reserve(92);
        std::stack<detail::iter, std::vector<detail::iter>> stk;
        stk.emplace(init_grid, 0);
        detail::queens_helper(stk, res);
        return res; // RVO
    }

    /**
     * @brief Solves the 8-Queens problem and returns only unique solutions under symmetry.
     *
     * Uses canonical symmetry reduction to eliminate equivalent boards.
     *
     * @return Unordered set of unique 8-Queens solutions
     */
    [[maybe_unused]] std::unordered_set<grid> queens_problem_uniq() {
        const auto all = queens_problem();
        std::unordered_set<grid> res;
        for (const auto g: all) {
            res.emplace(canonical(g));
        }
        return res; // RVO
    }
    /**
     * @brief Convert a bitboard grid to a human-readable string representation.
     *
     * Example:
     * . . . Q . . . .
     * . . . . . . Q .
     * Q . . . . . . .
     * . . . . . . . Q
     * . . . . Q . . .
     * . Q . . . . . .
     * . . . . . Q . .
     * . . Q . . . . .
     *
     * @param g Bitboard grid representing an 8x8 chessboard
     * @return Formatted string with Q for queens and . for empty spaces
     */
    [[maybe_unused]] inline std::string to_string(grid g) {
        std::string s;
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                s += ((g >> (row * 8 + col)) & 1ULL) ? 'Q' : '.';
                s += ' ';
            }
            s += '\n';
        }
        return s;
    }

} // namespace queens

