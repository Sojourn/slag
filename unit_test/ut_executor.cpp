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

struct SpinTask : Task {
    Event event;
    int   count = 100;
    int   index = 0;

    SpinTask() {
        event.set();
    }

    void run() override {
        index += 1;
        if (index == count) {
            set_success();
        }
    }

    Event& runnable_event() override {
        return event;
    }
};

using Connection = std::array<PollableQueue<int>, 2>;

struct PingTask : Task {
    PollableQueue<int>& rx_queue;
    PollableQueue<int>& tx_queue;
    int                 result;

    explicit PingTask(Connection& connection)
        : rx_queue{connection[0]}
        , tx_queue{connection[1]}
        , result{-1}
    {
    }

    void ping(int number) {
        tx_queue.push_back(number);
    }

    void run() override {
        result = *rx_queue.pop_front();
        set_success();
    }

    Event& runnable_event() override {
        return rx_queue.readable_event();
    }
};

struct PongTask : Task {
    PollableQueue<int>& rx_queue;
    PollableQueue<int>& tx_queue;

    explicit PongTask(Connection& connection)
        : rx_queue{connection[1]}
        , tx_queue{connection[0]}
    {
    }

    void run() override {
        tx_queue.push_back(*rx_queue.pop_front());
        set_success();
    }

    Event& runnable_event() override {
        return rx_queue.readable_event();
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

    SECTION("Spinning") {
        SpinTask spin_task;

        Executor executor;
        executor.insert(spin_task);
        while (executor.is_runnable()) {
            executor.run();
        }

        CHECK(spin_task.state() == TaskState::SUCCESS);
    }

    SECTION("PingPong") {
        Executor executor;

        Connection connection;
        PingTask ping_task{connection};
        PongTask pong_task{connection};

        executor.insert(ping_task);
        executor.insert(pong_task);

        CHECK(!executor.is_runnable());
        ping_task.ping(37);
        CHECK(executor.is_runnable());

        while (executor.is_runnable()) {
            executor.run();
        }

        CHECK(ping_task.result == 37);
    }
}
