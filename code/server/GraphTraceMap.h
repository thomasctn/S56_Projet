#pragma once
#include "../common/Constants.h"

#include <unordered_map>
#include <vector>
#include <cstdint>

struct Node;

enum class TraceType {
    PacMan,
    Ghost
};

struct Trace {
    TraceType type;
    uint32_t ownerId;
    float intensity;
};

class GraphTraceMap {
public:
    void add(Node* node, const Trace& trace);
    const std::vector<Trace>& get(Node* node) const;
    void decay();
    const std::unordered_map<Node*, std::vector<Trace>>& getAll() const { return traces; }

private:
    std::unordered_map<Node*, std::vector<Trace>> traces;
};
