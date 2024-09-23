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
        if (!has_edge(edge)) {
            return std::nullopt;
        }

        size_t result = 0;

        // Count the number of edges in preceding rows.
        for (ThreadIndex thread_index = 0; thread_index < edge.source; ++thread_index) {
            result += __builtin_popcountll(adjacent_nodes(thread_index));
        }

        // Count the number of edges in preceding columns.
        result += __builtin_popcountll(adjacent_nodes(edge.source) & ((1ull << edge.target) - 1));

        return result;
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

    ThreadIndex ThreadRoute::hop(const size_t index) const {
        if (hops_.size() <= index) {
            throw std::runtime_error("Hop index out-of-bounds");
        }

        return hops_[index];
    }

    ThreadIndex ThreadRoute::first_hop() const {
        return hops_[0];
    }

    // NOTE: This can be done in constant time with SIMD.
    ThreadIndex ThreadRoute::next_hop(ThreadIndex current) const {
        for (size_t i = 0; i < (hops_.size() - 1); ++i) {
            if (hops_[i] == current) {
                return hops_[i + 1];
            }
        }

        return hops_[0];
    }

    void ThreadRoute::add_hop(ThreadIndex next) {
        for (size_t i = 0; i < hops_.size(); ++i) {
            if (hops_[i] == INVALID_THREAD_INDEX) {
                hops_[i] = next;
                return;
            }
        }

        throw std::runtime_error("Too many hops");
    }

    ThreadRouteTable build_thread_route_table(const ThreadGraph& graph, const ThreadIndex origin) {
        size_t route_costs[MAX_THREAD_COUNT];
        for (size_t& cost : route_costs) {
            cost = std::numeric_limits<size_t>::max();
        }

        // The origin costs nothing to route to.
        route_costs[origin] = 0;

        ThreadMask working_set = (1ull << origin);
        ThreadMask visited_set = 0;

        ThreadRouteTable routes;
        while (working_set) {
            const ThreadIndex source = static_cast<ThreadIndex>(__builtin_ctzll(working_set));
            working_set &= ~(1ull << source);
            visited_set |= (1ull << source);

            for_each_thread(graph.adjacent_nodes(source), [&](const ThreadIndex target) {
                const size_t edge_cost = 1;
                const size_t source_route_cost = route_costs[source];
                const size_t proposed_route_cost = source_route_cost + edge_cost;

                // Update the best route to the target if we found a cheaper one.
                size_t& target_route_cost = route_costs[target];
                if (proposed_route_cost < target_route_cost) {
                    ThreadRoute new_route = routes[source];
                    new_route.add_hop(target);

                    routes[target] = new_route;
                    target_route_cost = source_route_cost + edge_cost;
                }

                // Add the target to the working set if it hasn't been visited yet.
                working_set |= (1ull << target) & ~visited_set;
            });
        }

        if (const ThreadMask nodes = graph.nodes(); nodes && nodes != visited_set) {
            throw std::runtime_error("Unreachable thread detected");
        }

        return routes;
    }

}
