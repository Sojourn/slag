#include <array>
#include "catch.hpp"
#include "slag/slag.h"

using namespace slag::postal;

struct MockTask : Task {
    int                  state = 0;
    std::array<Event, 4> events;

    void run() override {
        assert(events[state].is_set());
        state += 1;
    }

    Event& runnable_event() override {
        return events[state];
    }
};

TEST_CASE("Executor") {
    SECTION("Runnyness") {
        Executor executor;

        // The executor should have nothing to do.
        CHECK(!executor.runnable_event().is_set());

        MockTask mock_task;
        executor.insert(mock_task);

        while (mock_task.state < 3) {
            // The executor should not be runnable 
            CHECK(!executor.runnable_event().is_set());

            // Set the event and ensure the executor is runnable now.
            mock_task.events[mock_task.state].set();
            CHECK(executor.runnable_event().is_set());

            // Progress.
            executor.run();
        }
    }
}
