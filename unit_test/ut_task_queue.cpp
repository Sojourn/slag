#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

class DummyTask : public Task {
public:
    using Task::Task;

    void run() override {
        // pass
    }
};

TEST_CASE("wrapping", "[TaskQueue]") {
    Executor executor;

    DummyTask task0{executor};
    DummyTask task1{executor};
    DummyTask task2{executor};
    DummyTask task3{executor};

    Task* tasks[] = {
        &task0,
        &task1,
        &task2,
        &task3,
    };

    TaskQueue task_queue{4};
    CHECK(task_queue.size() == 0);
    CHECK(task_queue.capacity() == 4);

    SECTION("forwards") {
        for (size_t i = 0; i < 16; ++i) {
            task_queue.push_back(*tasks[i % 4]);
            CHECK(task_queue.size() == 1);
            CHECK(task_queue.pop_front() == tasks[i % 4]);
        }

        CHECK(task_queue.capacity() == 4);
    }

    SECTION("backwards") {
        for (size_t i = 0; i < 16; ++i) {
            task_queue.push_front(*tasks[i % 4]);
            CHECK(task_queue.size() == 1);
            CHECK(task_queue.pop_back() == tasks[i % 4]);
        }

        CHECK(task_queue.capacity() == 4);
    }
}
