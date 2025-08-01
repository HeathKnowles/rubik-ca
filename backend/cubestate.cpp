#include "cubestate.h"
#include "movetables.h"

#include <array>
#include <sstream>
#include <bitset>

constexpr uint64_t MASK5 = 0b11111;

inline uint64_t get5Bits(uint64_t state, int index) {
    return (state >> (index * 5)) & MASK5;
}

inline void set5Bits(uint64_t &state, int index, uint64_t value) {
    state &= ~(MASK5 << (index * 5));
    state |= (value & MASK5) << (index * 5);
}

CubeState::CubeState() : cornerData(0), edgeData(0) {
    // Identity permutation (solved cube)
    for (int i = 0; i < 8; ++i) {
        set5Bits(cornerData, i, (i << 2)); // orientation = 0
    }
    for (int i = 0; i < 12; ++i) {
        set5Bits(edgeData, i, (i << 1)); // flip = 0
    }
}

void CubeState::applyMove(int moveIndex) {
    uint64_t newCorners = 0;
    for (int i = 0; i < 8; ++i) {
        int src = cornerPerm[moveIndex][i];
        uint64_t corner = get5Bits(cornerData, src);
        int orient = corner & 0b11;
        int pos = corner >> 2;
        int delta = cornerOrientDelta[moveIndex][i];
        orient = (orient + delta) % 3;
        set5Bits(newCorners, i, (pos << 2) | orient);
    }
    cornerData = newCorners;

    uint64_t newEdges = 0;
    for (int i = 0; i < 12; ++i) {
        int src = edgePerm[moveIndex][i];
        uint64_t edge = get5Bits(edgeData, src);
        int flip = edge & 0b1;
        int pos = edge >> 1;
        int delta = edgeFlipDelta[moveIndex][i];
        flip ^= delta;
        set5Bits(newEdges, i, (pos << 1) | flip);
    }
    edgeData = newEdges;
}

std::string CubeState::getCornerPermutation() const {
    std::ostringstream oss;
    for (int i = 0; i < 8; ++i) {
        oss << (get5Bits(cornerData, i) >> 2);
        if (i != 7) oss << " ";
    }
    return oss.str();
}

std::string CubeState::getEdgePermutation() const {
    std::ostringstream oss;
    for (int i = 0; i < 12; ++i) {
        oss << (get5Bits(edgeData, i) >> 1);
        if (i != 11) oss << " ";
    }
    return oss.str();
}

bool CubeState::isSolved() const {
    for (int i = 0; i < 8; ++i) {
        if (get5Bits(cornerData, i) != (i << 2))
            return false;
    }
    for (int i = 0; i < 12; ++i) {
        if (get5Bits(edgeData, i) != (i << 1))
            return false;
    }
    return true;
}

std::string CubeState::toBitString() const {
    std::bitset<64> cBits(cornerData);
    std::bitset<64> eBits(edgeData);
    return cBits.to_string() + " | " + eBits.to_string();
}
