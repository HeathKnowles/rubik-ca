#pragma once

#include <array>
#include <string>

// Number of standard moves
constexpr int NUM_MOVES = 18;

// Move tables declarations
extern const std::array<std::array<int, 8>, NUM_MOVES> cornerPerm;
extern const std::array<std::array<int, 8>, NUM_MOVES> cornerOrientDelta;

extern const std::array<std::array<int, 12>, NUM_MOVES> edgePerm;
extern const std::array<std::array<int, 12>, NUM_MOVES> edgeFlipDelta;

// New: inverse moves table - maps move index to its inverse move index
extern const std::array<int, NUM_MOVES> moveInverse;

// New: human-readable move names corresponding to move indices
extern const std::array<std::string, NUM_MOVES> moveNames;
