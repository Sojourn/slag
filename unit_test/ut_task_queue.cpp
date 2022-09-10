#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

TEST_CASE("wrapping", "[TaskQueue]") {
    Task tasks[4];

    TaskQueue task_queue{4};
    CHECK(task_queue.size() == 0);
    CHECK(task_queue.capacity() == 4);

    SECTION("forwards") {
        for (size_t i = 0; i < 16; ++i) {
            task_queue.push_back(tasks[i % 4]);
            CHECK(task_queue.size() == 1);
            CHECK(task_queue.pop_front() == &tasks[i % 4]);
        }

        CHECK(task_queue.capacity() == 4);
    }

    SECTION("backwards") {
        for (size_t i = 0; i < 16; ++i) {
            task_queue.push_front(tasks[i % 4]);
            CHECK(task_queue.size() == 1);
            CHECK(task_queue.pop_back() == &tasks[i % 4]);
        }

        CHECK(task_queue.capacity() == 4);
    }
}
