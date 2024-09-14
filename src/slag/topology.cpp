#include "slag/topology.h"
#include <cassert>

namespace slag {

    ThreadMask ThreadGraph::nodes() const {
        ThreadMask result = 0;

        for (ThreadIndex thread_index = 0; thread_index < MAX_THREAD_COUNT; ++thread_index) {
            if (adjacency_matrix_[thread_index]) {
                result |= (1ull << thread_index);
            }
        }

        return result;
    }

    ThreadMask ThreadGraph::adjacent_nodes(const ThreadIndex thread_index) const {
        assert(thread_index < MAX_THREAD_COUNT);

        return adjacency_matrix_[thread_index];
    }

    size_t ThreadGraph::edge_count() const {
        size_t result = 0;

        for (ThreadIndex thread_index = 0; thread_index < MAX_THREAD_COUNT; ++thread_index) {
            result += __builtin_popcountll(adjacency_matrix_[thread_index]);
        }

        return result;
    }

    std::optional<size_t> ThreadGraph::edge_index(const Edge edge) const {
        size_t result = 0;

        // Count the number of edges in preceding rows.
        for (ThreadIndex thread_index = 0; thread_index < edge.source; ++thread_index) {
            result += __builtin_popcountll(adjacent_nodes(thread_index));
        }

        // Count the number of edges in preceding columns.
        for (ThreadIndex thread_index = 0; thread_index < edge.target; ++thread_index) {
            if (adjacency_matrix_[edge.source] & (1ull << thread_index)) {
                result += 1;
            }
        }

        return std::nullopt;
    }

    bool ThreadGraph::has_edge(const Edge edge) const {
        assert(edge.source < MAX_THREAD_COUNT);
        assert(edge.target < MAX_THREAD_COUNT);

        return static_cast<bool>(adjacency_matrix_[edge.source] & (1ull << edge.target));
    }

    void ThreadGraph::add_edge(const Edge edge) {
        assert(edge.source < MAX_THREAD_COUNT);
        assert(edge.target < MAX_THREAD_COUNT);

        if (edge.source == edge.target) {
            throw std::runtime_error("Self-edges are not supported");
        }

        adjacency_matrix_[edge.source] |= (1ull << edge.target);
    }

    ThreadRoute::ThreadRoute() {
        for (ThreadIndex& hop : hops_) {
            hop = INVALID_THREAD_INDEX;
        }
    }

    std::optional<ThreadIndex> ThreadRoute::first_hop() const {
        if (hops_[0] != INVALID_THREAD_INDEX) {
            return hops_[0];
        }

        return std::nullopt;
    }

    // NOTE: This can be done in constant time with SIMD.
    std::optional<ThreadIndex> ThreadRoute::next_hop(ThreadIndex current) const {
        for (size_t i = 0; i < (hops_.size() - 1); ++i) {
            if (hops_[i] == current) {
                return hops_[i + 1];
            }
        }

        return std::nullopt;
    }

    // NOTE: This can be done in constant time with SIMD.
    void ThreadRoute::add_hop(ThreadIndex next) {
        for (size_t i = 0; i < hops_.size(); ++i) {
            if (hops_[i] == INVALID_THREAD_INDEX) {
                hops_[i] = next;
                return;
            }
        }

        throw std::runtime_error("Too many hops on thread route");
    }

}
