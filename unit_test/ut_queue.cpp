#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

TEST_CASE("Queue") {
    Queue<int> queue;

    SECTION("Pushing and popping elements") {
        for (int i = 0; i < 1000; ++i) {
            queue.push_back(i);
        }

        for (int i = 0; i < 1000; ++i) {
            CHECK(queue.pop_front() == i);
        }

        CHECK(queue.is_empty());
        CHECK(queue.pop_front() == std::nullopt);
    }

    SECTION("Index based element access") {
        for (int i = 0; i < 1000; ++i) {
            queue.push_back(i);
        }

        for (int i = 0; i < 1000; ++i) {
            CHECK(queue[static_cast<size_t>(i)] == i);
        }
    }

    SECTION("Move construction and assignment") {
        for (int i = 0; i < 1000; ++i) {
            queue.push_back(i);
        }

        // move construct into a temporary queue
        Queue<int> temp_queue(std::move(queue));
        CHECK(queue.is_empty());
        CHECK(queue.pop_front() == std::nullopt);
        for (int i = 0; i < 1000; ++i) {
            CHECK(temp_queue[static_cast<size_t>(i)] == i);
        }

        // move assign back into the original queue
        queue = std::move(temp_queue);
        CHECK(temp_queue.is_empty());
        CHECK(temp_queue.pop_front() == std::nullopt);
        for (int i = 0; i < 1000; ++i) {
            CHECK(queue[static_cast<size_t>(i)] == i);
        }
    }
}
