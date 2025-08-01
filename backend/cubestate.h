// CubeState.h
#ifndef CUBESTATE_H
#define CUBESTATE_H

#include <cstdint>
#include <bitset>
#include <string>

class CubeState {
public:
    // Each corner: 3 bits for position, 2 bits for orientation = 5 bits × 8 = 40 bits
    // Each edge: 4 bits for position, 1 bit for orientation = 5 bits × 12 = 60 bits
    // Total: 100 bits (use two uint64_t to cover this)

    uint64_t cornerData;  // Stores all 8 corners (position + orientation)
    uint64_t edgeData;    // Stores all 12 edges (position + orientation)

    CubeState();

    // Apply move based on predefined index (0-17 for standard moves)
    void applyMove(int moveIndex);

    // Get corner permutation (0-7) as a string or vector for external use
    std::string getCornerPermutation() const;

    // Get edge permutation (0-11) as a string or vector for external use
    std::string getEdgePermutation() const;

    // Check if cube is in solved state
    bool isSolved() const;

    // Print cube state as binary string (for debugging)
    std::string toBitString() const;
};

#endif // CUBESTATE_H
