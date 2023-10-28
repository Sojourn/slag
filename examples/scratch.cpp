#include <iostream>
#include "slag/core.h"
#include "slag/system.h"
#include "slag/scheduling.h"
#include "slag/event_loop.h"

using namespace slag;

class TickTask : public ProtoTask {
public:
    TickTask()
        : ProtoTask{TaskPriority::HIGH}
    {
    }

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

template<typename TaskImpl, typename... Args>
void run_until_complete(ServiceRegistry& service_registry, Args&&... args) {
}

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

    ServiceRegistry service_registry;
    SystemService system_service{service_registry};
    SchedulerService scheduler_service{service_registry};
    EventLoop event_loop{service_registry};

    auto init_task = std::make_unique<TickTask>();
    // auto shutdown_task = std::make_unique<ShutdownTask>(event_loop);

    scheduler_service.schedule_task(*init_task);

    event_loop.loop();

    init_task.reset();
    // shutdown_task.reset();


    return 0;
}
