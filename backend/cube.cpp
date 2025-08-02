#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <queue>
#include <random>
#include <functional>
#include <chrono>
#include <sstream>
#include <string>
#include <emscripten.h>

enum Face { U, D, F, B, L, R };
const int NUM_FACES = 6;

struct Move {
    Face face;
    int turns;
    Move(Face f = U, int t = 1) : face(f), turns(t) {}
};

// Cache all moves once
const std::vector<Move>& allMoves() {
    static const std::vector<Move> moves = [] {
        std::vector<Move> mv;
        for (int f = 0; f < NUM_FACES; ++f)
            for (int t = 1; t <= 3; ++t)
                mv.emplace_back((Face)f, t);
        return mv;
    }();
    return moves;
}

struct CubeState {
    std::array<uint8_t, 12> edges{};
    std::array<uint8_t, 8> corners{};
    std::array<uint8_t, 12> edge_orient{};
    std::array<uint8_t, 8> corner_orient{};

    bool operator==(const CubeState& o) const {
        return edges == o.edges && corners == o.corners &&
               edge_orient == o.edge_orient && corner_orient == o.corner_orient;
    }
};

namespace std {
    template <> struct hash<CubeState> {
        size_t operator()(const CubeState& c) const {
            size_t h = 0;
            for (auto x : c.edges) h = h * 31 + x;
            for (auto x : c.corners) h = h * 31 + x;
            for (auto x : c.edge_orient) h = h * 31 + x;
            for (auto x : c.corner_orient) h = h * 31 + x;
            return h;
        }
    };

    template <> struct hash<std::array<uint8_t, 12>> {
        size_t operator()(const std::array<uint8_t, 12>& a) const {
            size_t h = 0; for (auto x : a) h = h * 31 + x; return h;
        }
    };

    template <> struct hash<std::array<uint8_t, 8>> {
        size_t operator()(const std::array<uint8_t, 8>& a) const {
            size_t h = 0; for (auto x : a) h = h * 31 + x; return h;
        }
    };

    template <> struct hash<std::array<bool, 12>> {
        size_t operator()(const std::array<bool, 12>& a) const {
            size_t h = 0; for (bool x : a) h = h * 2 + x; return h;
        }
    };
}

CubeState solvedCube() {
    CubeState c;
    for (int i = 0; i < 12; ++i) c.edges[i] = i;
    for (int i = 0; i < 8; ++i) c.corners[i] = i;
    return c;
}

struct MoveTables {
    std::array<std::vector<int>, 6> edge_cycles {{
        {0,1,2,3}, {4,5,6,7}, {2,6,10,7},
        {0,5,8,4}, {3,7,11,4}, {1,6,9,5}
    }};
    std::array<std::vector<int>, 6> corner_cycles {{
        {0,1,2,3}, {4,5,6,7}, {1,5,6,2},
        {0,3,7,4}, {0,4,5,1}, {2,6,7,3}
    }};
    std::array<std::vector<int>, 6> corner_orient {{
        {0,0,0,0}, {0,0,0,0}, {1,2,1,2},
        {2,1,2,1}, {1,2,1,2}, {2,1,2,1}
    }};
    std::array<std::vector<int>, 6> edge_orient {{
        {0,0,0,0}, {0,0,0,0}, {1,0,1,0},
        {1,0,1,0}, {0,0,0,0}, {0,0,0,0}
    }};

    CubeState apply(const CubeState& c, Face face, int turns) {
        CubeState r = c;
        while (turns--) {
            auto& e = edge_cycles[face];
            uint8_t te = r.edges[e[3]], to = r.edge_orient[e[3]];
            for (int i = 3; i > 0; --i) {
                r.edges[e[i]] = r.edges[e[i - 1]];
                r.edge_orient[e[i]] = (r.edge_orient[e[i - 1]] + edge_orient[face][i]) % 2;
            }
            r.edges[e[0]] = te;
            r.edge_orient[e[0]] = (to + edge_orient[face][0]) % 2;

            auto& cr = corner_cycles[face];
            uint8_t tc = r.corners[cr[3]], tco = r.corner_orient[cr[3]];
            for (int i = 3; i > 0; --i) {
                r.corners[cr[i]] = r.corners[cr[i - 1]];
                r.corner_orient[cr[i]] = (r.corner_orient[cr[i - 1]] + corner_orient[face][i]) % 3;
            }
            r.corners[cr[0]] = tc;
            r.corner_orient[cr[0]] = (tco + corner_orient[face][0]) % 3;
        }
        return r;
    }
};

MoveTables moveTables;

CubeState applyMove(const CubeState& c, const Move& m) {
    return moveTables.apply(c, m.face, m.turns);
}

template<typename Key>
std::unordered_map<Key, int> generatePDB(const CubeState& start, std::function<Key(const CubeState&)> extractor, int max_depth) {
    std::unordered_map<Key, int> pdb;
    std::queue<std::pair<CubeState, int>> q;
    pdb.emplace(extractor(start), 0);
    q.push({start, 0});
    const auto& moves = allMoves();

    while (!q.empty()) {
        auto [state, depth] = q.front(); q.pop();
        if (depth >= max_depth) continue;
        for (const auto& m : moves) {
            CubeState next = applyMove(state, m);
            Key k = extractor(next);
            auto [it, inserted] = pdb.emplace(k, depth + 1);
            if (inserted) {
                q.push({next, depth + 1});
            }
        }
    }
    return pdb;
}

std::array<uint8_t, 12> getEdgeOrientation(const CubeState& c) {
    return c.edge_orient;
}

std::array<uint8_t, 8> getCornerOrientation(const CubeState& c) {
    return c.corner_orient;
}

std::array<bool, 12> getESlice(const CubeState& c) {
    std::array<bool, 12> slice{};
    for (int i = 0; i < 12; ++i) {
        slice[i] = (c.edges[i] >= 4 && c.edges[i] <= 7);  // Middle layer edge IDs
    }
    return slice;
}

struct IDASolver {
    CubeState goal{};
    std::unordered_map<std::array<uint8_t, 12>, int> edgePDB;
    std::unordered_map<std::array<uint8_t, 8>, int> cornerPDB;
    std::unordered_map<std::array<bool, 12>, int> eslicePDB;

    size_t nodes_visited = 0;
    size_t node_limit = 50'000'000;

    int heuristic(const CubeState& c) {
        auto edgeOri = getEdgeOrientation(c);
        auto cornerOri = getCornerOrientation(c);
        auto eSlice = getESlice(c);
        int edgeH = 0, cornerH = 0, esliceH = 0;

        auto edgeIt = edgePDB.find(edgeOri);
        if (edgeIt != edgePDB.end()) edgeH = edgeIt->second;

        auto cornerIt = cornerPDB.find(cornerOri);
        if (cornerIt != cornerPDB.end()) cornerH = cornerIt->second;

        auto eSliceIt = eslicePDB.find(eSlice);
        if (eSliceIt != eslicePDB.end()) esliceH = eSliceIt->second;

        return std::max({edgeH, cornerH, esliceH});
    }

    bool dfs(CubeState c, int g, int threshold, std::vector<Move>& path) {
        if (++nodes_visited > node_limit) return false;

        int f = g + heuristic(c);
        if (f > threshold) return false;
        if (c == goal) return true;

        const auto& moves = allMoves();

        for (const auto& m : moves) {
            if (!path.empty()) {
                auto& last = path.back();
                if (last.face == m.face) continue;

                if (path.size() >= 2 && path[path.size() - 2].face == m.face &&
                    (last.turns + m.turns) % 4 == 0) continue;
            }

            CubeState next = applyMove(c, m);
            path.push_back(m);
            if (dfs(next, g + 1, threshold, path))
                return true;
            path.pop_back();
        }
        return false;
    }

    std::vector<Move> solve(const CubeState& start) {
        int threshold = heuristic(start);
        std::vector<Move> path;

        while (threshold <= 30) {
            std::cout << "Trying threshold: " << threshold << "\n";
            nodes_visited = 0;
            if (dfs(start, 0, threshold, path)) return path;
            threshold++;
        }
        return {};
    }
};

void scrambleCube(CubeState& c, int n = 14) {
    const auto& moves = allMoves();
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, (int)moves.size() - 1);
    Face last = U;

    for (int i = 0; i < n; ++i) {
        Move m;
        do { m = moves[dist(rng)]; } while (m.face == last);
        last = m.face;
        c = applyMove(c, m);
        const char* f = "UDFBLR";
        std::string suffix = (m.turns == 1 ? "" : m.turns == 2 ? "2" : "'");
        std::cout << f[m.face] << suffix << " ";
    }
    std::cout << "\n";
}

Move parseMove(const std::string& token) {
    const char* faceChars = "UDFBLR";
    Face face = U;
    int turns = 1;

    if (token.empty()) return {U, 1};

    auto pos = std::string(faceChars).find(token[0]);
    if (pos != std::string::npos) face = static_cast<Face>(pos);

    if (token.size() > 1) {
        if (token[1] == '2') turns = 2;
        else if (token[1] == '\'') turns = 3;
    }

    return {face, turns};
}

void applyScramble(CubeState& c, const std::string& input) {
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        Move m = parseMove(token);
        c = applyMove(c, m);
    }
}

// int main(int argc, char** argv) {
//     if (argc < 2) {
//         std::cerr << "Usage: ./solver \"SCRAMBLE\"\n";
//         return 1;
//     }

//     std::string scramble = argv[1];
//     CubeState start = solvedCube();
//     applyScramble(start, scramble);

//     IDASolver solver;
//     solver.goal = solvedCube();
//     solver.edgePDB = generatePDB<std::array<uint8_t, 12>>(solver.goal, getEdgeOrientation, 14);
//     solver.cornerPDB = generatePDB<std::array<uint8_t, 8>>(solver.goal, getCornerOrientation, 14);
//     solver.eslicePDB = generatePDB<std::array<bool, 12>>(solver.goal, getESlice, 14);

//     auto solution = solver.solve(start);
//     const char* faceNames = "UDFBLR";
//     for (auto& move : solution) {
//         std::string suffix = (move.turns == 1 ? "" : move.turns == 2 ? "2" : "'");
//         std::cout << faceNames[move.face] << suffix << " ";
//     }

//     std::cout << std::endl;
//     return 0;
// }

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    const char* solveCube(const char* scramble) {
        static std::string result;

        CubeState start = solvedCube();
        applyScramble(start, scramble);

        IDASolver solver;
        solver.goal = solvedCube();
        solver.edgePDB = generatePDB<std::array<uint8_t, 12>>(solver.goal, getEdgeOrientation, 14);
        solver.cornerPDB = generatePDB<std::array<uint8_t, 8>>(solver.goal, getCornerOrientation, 14);
        solver.eslicePDB = generatePDB<std::array<bool, 12>>(solver.goal, getESlice, 14);

        auto solution = solver.solve(start);

        const char* faceNames = "UDFBLR";
        std::ostringstream oss;
        for (auto& move : solution) {
            std::string suffix = (move.turns == 1 ? "" : move.turns == 2 ? "2" : "'");
            oss << faceNames[move.face] << suffix << " ";
        }

        result = oss.str();
        return result.c_str(); // safe as long as result stays in scope
    }
}