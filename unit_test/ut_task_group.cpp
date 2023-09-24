#include <array>
#include "catch.hpp"
#include "slag/slag.h"
#include "slag/core/task.h"
#include "slag/core/task/task_group.h"
#include "slag/core/executor.h"

using namespace slag;

class SquareTask : public Task {
public:
    explicit SquareTask(int number)
        : number_{number}
        , result_{-1}
    {
        event_.set();
    }

    int result() const {
        assert(is_success());
        return result_;
    }

    Event& runnable_event() {
        return event_;
    }

    void run() override {
        result_ = number_ * number_;
        set_success();
    }

private:
    Event event_;
    int   number_;
    int   result_;
};

// Run a group of square tasks concurrently.
class Squarallel : public TaskGroup<SquareTask> {
public:
    Squarallel(std::span<int> numbers)
        : remaining_tasks_{numbers.size()}
    {
        for (int number: numbers) {
            spawn(number);
        }
    }

    std::span<const int> result() const {
        assert(is_success());
        return {
            result_.data(),
            result_.size(),
        };
    }

private:
    void reap(SquareTask& task) {
        if (task.is_failure()) {
            Task::set_failure();
            return;
        }

        result_.push_back(task.result());

        remaining_tasks_ -= 1;
        if (remaining_tasks_ == 0) {
            Task::set_success();
        }
    }

    std::vector<int> result_;
    size_t           remaining_tasks_;
};

class LeafTask : public Task {
public:
    LeafTask() {
        runnable_event_.set();
    }

    Event& runnable_event() override {
        return runnable_event_;
    }

    void run() override {
        set_success();
    }

private:
    Event runnable_event_;
};

class BranchTask : public Task {
public:
    BranchTask()
        : state_{0}
    {
        runnable_event_.set();
    }

    Event& runnable_event() {
        switch (state_) {
            case 0: {
                return runnable_event_;
            }
            case 1: {
                return leaf_task_.complete_event();
            }
            default: {
                abort();
            }
        }
    }

    void run() override {
        switch (state_++) {
            case 0: {
                // Add the leaf task to our parent executor.
                region().current_executor().insert(leaf_task_);
                break;
            }
            case 1: {
                set_success(leaf_task_.is_success());
                break;
            }
            default: {
                abort();
            }
        }
    }

private:
    Event    runnable_event_;
    LeafTask leaf_task_;
    int      state_;
};

TEST_CASE("TaskGroup") {
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

    SECTION("Mapping") {
        Executor executor;

        std::array<int, 4> numbers;
        numbers[0] = 1;
        numbers[1] = 2;
        numbers[2] = 3;
        numbers[3] = 4;

        Squarallel task{{numbers.data(), numbers.size()}};
        executor.insert(task);
        while (executor.is_runnable()) {
            executor.run();
        }

        CHECK(task.is_success() == true);
        CHECK(task.result()[0] == 1);
        CHECK(task.result()[1] == 4);
        CHECK(task.result()[2] == 9);
        CHECK(task.result()[3] == 16);
    }

    SECTION("Tree") {
        Executor executor;

        TaskGroup<BranchTask> task_group;
        task_group.spawn();
        task_group.spawn();
        task_group.spawn();
        CHECK(task_group.size() == 3);

        executor.insert(task_group);
        while (executor.is_runnable()) {
            executor.run();
        }

        CHECK(task_group.size() == 0);
    }
}
