#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

class TestTask : public Task {
public:
    using Task::Task;

    void run() override {
        ++activations;
    }

    size_t activations = 0;
};

TEST_CASE("basic", "[Executor]") {
    Executor executor;
    TestTask task1{executor};
    TestTask task2{executor};

    SECTION("recursive scheduling") {
        // TODO
    }

    SECTION("cancelation") {
        task1.schedule();
        task1.cancel();

        executor.run(1);
        CHECK(executor.is_idle());
        CHECK(task1.activations == 0);
    }

    SECTION("reprioritization") {
        task1.schedule(TaskPriority::NORMAL); // 1
        task2.schedule(TaskPriority::NORMAL); // 1 2
        task2.schedule(TaskPriority::HIGH);   // 2 1 X

        // run task2
        executor.run(1);
        CHECK(task1.activations == 0);
        CHECK(task2.activations == 1);

        CHECK(!executor.is_idle());

        // run task1
        executor.run(1);
        CHECK(task1.activations == 1);
        CHECK(task2.activations == 1);

        CHECK(!executor.is_idle());

        // discard the tombstone
        executor.run(1);
        CHECK(task1.activations == 1);
        CHECK(task2.activations == 1);
    }
}
