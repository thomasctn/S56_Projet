#include "GraphTraceMap.h"
#include <algorithm>

void GraphTraceMap::add(Node* node, const Trace& trace) {
    traces[node].push_back(trace);
}

const std::vector<Trace>& GraphTraceMap::get(Node* node) const {
    static const std::vector<Trace> empty;
    auto it = traces.find(node);
    return (it != traces.end()) ? it->second : empty;
}

void GraphTraceMap::decay() {
    for (auto& [node, list] : traces) {
        for (auto& t : list)
            t.intensity *= DECAY_FACTOR;

        list.erase(
            std::remove_if(
                list.begin(),
                list.end(),
                [](const Trace& t) {
                    return t.intensity <= 0.f;
                }
            ),
            list.end()
        );
    }
}
