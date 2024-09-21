#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

TEST_CASE("Topology") {
    ThreadGraph graph;

    SECTION("Unconnected network") {
        CHECK(graph.nodes() == 0);
        CHECK(graph.adjacent_nodes(0) == 0);
        CHECK(graph.edge_count() == 0);

        for (ThreadRoute route : build_thread_route_table(graph, MAX_THREAD_COUNT)) {
            CHECK(!route.first_hop());
            for (ThreadIndex thread_index = 0; thread_index < MAX_THREAD_COUNT; ++thread_index) {
                CHECK(!route.next_hop(thread_index));
            }
        }
    }

    SECTION("Ring network") {
        graph.add_edge({0, 1});
        graph.add_edge({1, 2});
        graph.add_edge({2, 3});
        graph.add_edge({3, 0});

        CHECK(graph.edge_index({0, 1}) == 0);
        CHECK(graph.edge_index({1, 2}) == 1);
        CHECK(graph.edge_index({2, 3}) == 2);
        CHECK(graph.edge_index({3, 0}) == 3);

        CHECK(graph.adjacent_nodes(0) == (1ull << 1));
        CHECK(graph.adjacent_nodes(1) == (1ull << 2));
        CHECK(graph.adjacent_nodes(2) == (1ull << 3));
        CHECK(graph.adjacent_nodes(3) == (1ull << 0));

        for (ThreadIndex origin = 0; origin < 4; ++origin) {
            ThreadRouteTable routes = build_thread_route_table(graph, origin);

            for (ThreadIndex target = 0; target < 4; ++target) {
                ThreadRoute route = routes[target];

                if (origin == target) {
                    CHECK(!route.first_hop());
                    CHECK(!route.next_hop(origin));
                }
                else {
                    auto next_index = [](ThreadIndex index) {
                        return (index + 1) % 4;
                    };

                    CHECK(route.first_hop() == next_index(origin));
                    for (ThreadIndex index = origin; index != target; index = next_index(index)) {
                        CHECK(route.next_hop(index) == next_index(index));
                    }

                    CHECK(route.next_hop(target) == INVALID_THREAD_INDEX);
                }
            }
        }
    }

    // TODO: Hypercube network
    // TODO: Fully connected network
}
