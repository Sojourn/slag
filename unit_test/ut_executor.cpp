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
    Empire::Config empire_config;
    empire_config.index = 0;

    Nation::Config nation_config;
    nation_config.index                 = 0;
    nation_config.buffer_count          = 1024;
    nation_config.region_count          = 1;
    nation_config.parcel_queue_capacity = 512;

    Region::Config region_config;
    region_config.index        = 0;
    region_config.buffer_range = std::make_pair(0, 1024);

    Empire empire_{empire_config};
    Nation nation_{nation_config};
    Region region_{region_config};

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
