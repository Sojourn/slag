#pragma once

#include <stdexcept>
#include <optional>
#include <array>
#include <limits>
#include <climits>
#include <cstdint>
#include <cstddef>

namespace slag {

    // TODO: Widen `ThreadIndex` to 16-bits.

    using ThreadIndex = uint8_t;
    using ThreadMask  = uint64_t;

    constexpr size_t MAX_THREAD_COUNT = 64;
    constexpr size_t INVALID_THREAD_INDEX = std::numeric_limits<ThreadIndex>::max();

    static_assert(MAX_THREAD_COUNT <= INVALID_THREAD_INDEX, "Use a wider type for ThreadIndex");
    static_assert(MAX_THREAD_COUNT <= (sizeof(ThreadMask) * CHAR_BIT), "Use a wider type for ThreadMask");

    class ThreadGraph {
    public:
        struct Edge {
            ThreadIndex source = INVALID_THREAD_INDEX;
            ThreadIndex target = INVALID_THREAD_INDEX;
        };

        ThreadMask nodes() const;
        ThreadMask adjacent_nodes(ThreadIndex thread_index) const;

        size_t edge_count() const;
        std::optional<size_t> edge_index(Edge edge) const;

        bool has_edge(Edge edge) const;
        void add_edge(Edge edge);

    private:
        ThreadMask adjacency_matrix_[MAX_THREAD_COUNT] = {0};
    };

    class ThreadRoute {
    public:
        ThreadRoute();

        std::optional<ThreadIndex> first_hop() const;
        std::optional<ThreadIndex> next_hop(ThreadIndex current) const;

        void add_hop(ThreadIndex next);

    private:
        std::array<ThreadIndex, 8> hops_;
    };

    using ThreadRouteTable = std::array<ThreadRoute, MAX_THREAD_COUNT>;

    template<typename Visitor>
    void for_each_thread(ThreadMask mask, Visitor&& visitor) {
        static_assert(sizeof(mask) == sizeof(uint64_t));

        const size_t count = __builtin_popcountll(mask);
        for (size_t i = 0; i < count; ++i) {
            const ThreadIndex thread_index = static_cast<ThreadIndex>(__builtin_ctzll(mask));
            mask &= ~(1ull << thread_index);
            visitor(thread_index);
        }
    }

    // Computes shortest paths from an origin thread to all other threads in a graph.
    // TODO: Add a cost model better than naive #hops.
    //       - NUMA node.
    //       - Shared L2/L3 caches.
    //       - CPU time to route away from (idle? busy?) threads.
    // TODO: Think about supporting multiple routes to a target and load balancing.
    //
    inline ThreadRouteTable build_thread_route_table(const ThreadGraph& graph, const ThreadIndex origin) {
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

        if (visited_set != graph.nodes()) {
            throw std::runtime_error("Unreachable thread detected");
        }

        return routes;
    }

}
