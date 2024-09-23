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

        ThreadIndex hop(size_t index) const;
        ThreadIndex first_hop() const;
        ThreadIndex next_hop(ThreadIndex current) const;

        void add_hop(ThreadIndex next);

    private:
        std::array<ThreadIndex, 4> hops_;
    };

    using ThreadRouteTable = std::array<ThreadRoute, MAX_THREAD_COUNT>;

    // Computes shortest paths from an origin thread to all other threads in a graph.
    // TODO: Add a cost model better than naive #hops.
    //       - NUMA node.
    //       - Shared L2/L3 caches.
    //       - CPU time to route away from (idle? busy?) threads.
    // TODO: Think about supporting multiple routes to a target and load balancing.
    //
    ThreadRouteTable build_thread_route_table(const ThreadGraph& graph, ThreadIndex origin);

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

}
