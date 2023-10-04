#include <iostream>
#include "slag/core.h"
#include "slag/system.h"
#include "slag/event_loop.h"

using namespace slag;

class InitTask : public ProtoTask {
public:
    void run() override final {
        SLAG_PT_BEGIN();

        while (true) {
            timer_operation_ = make_timer_operation(
                std::chrono::milliseconds(250)
            );

            SLAG_PT_WAIT_COMPLETE(*timer_operation_);
            if (timer_operation_->result()) {
                info("tick...");
            }
        }

        SLAG_PT_END();
    }

private:
    TimerOperationHandle timer_operation_;
};

class ShutdownTask : public ProtoTask {
public:
    explicit ShutdownTask(EventLoop& event_loop)
        : event_loop_{event_loop}
    {
    }

    void run() override final {
        SLAG_PT_BEGIN();

        timer_operation_ = make_timer_operation(std::chrono::seconds(5));
        SLAG_PT_WAIT_COMPLETE(*timer_operation_);
        assert(timer_operation_->result());

        event_loop_.stop();

        SLAG_PT_END();
    }

private:
    EventLoop&           event_loop_;
    TimerOperationHandle timer_operation_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Empire::Config empire_config;
    empire_config.index = 0;
    Empire empire_{empire_config};

    Nation::Config nation_config;
    nation_config.index                 = 0;
    nation_config.buffer_count          = 16 * 1024 + 1;
    nation_config.region_count          = 1;
    nation_config.parcel_queue_capacity = 512;
    Nation nation_{nation_config};

    Region::Config region_config;
    region_config.index = 0;
    region_config.buffer_range = std::make_pair(0, nation_config.buffer_count);
    Region region_{region_config};

    EventLoop event_loop;

    auto init_task = std::make_unique<InitTask>();
    auto shutdown_task = std::make_unique<ShutdownTask>(event_loop);

    event_loop.bind(*init_task);
    event_loop.bind(*shutdown_task);
    event_loop.loop();

    init_task.reset();
    shutdown_task.reset();

    return 0;
}
