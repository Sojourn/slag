#include <iostream>
#include "slag/core.h"
#include "slag/system.h"
#include "slag/memory.h"
#include "slag/scheduling.h"
#include "slag/event_loop.h"

using namespace slag;

class TickTask : public ProtoTask {
public:
    TickTask()
        : ProtoTask{TaskPriority::HIGH}
        , i_{0}
    {
    }

    void run() override final {
        SLAG_PT_BEGIN();

        for (i_ = 0; i_ < 4; ++i_) {
            timer_operation_ = make_timer_operation(
                std::chrono::milliseconds(250)
            );

            SLAG_PT_WAIT_COMPLETE(*timer_operation_);
            std::cout << "Tick" << std::endl;
            timer_operation_.reset();
        }

        SLAG_PT_END();
    }

private:
    TimerOperationHandle timer_operation_;
    size_t               i_;
};

class Stopper : public ProtoTask {
public:
    Stopper(EventLoop& event_loop, Task& task)
        : ProtoTask{TaskPriority::HIGH}
        , event_loop_{event_loop}
        , task_{task}
    {
    }

    void run() override final {
        SLAG_PT_BEGIN();

        SLAG_PT_WAIT_COMPLETE(task_);
        event_loop_.stop();

        SLAG_PT_END();
    }

private:
    EventLoop& event_loop_;
    Task&      task_;
};

template<typename TaskImpl, typename... Args>
void run_until_complete(ServiceRegistry& service_registry, Args&&... args) {
    EventLoop event_loop(service_registry);
    TaskImpl task(std::forward<Args>(args)...) ;
    Stopper stopper(event_loop, task);

    auto&& scheduler_service = get_scheduler_service(service_registry);
    scheduler_service.schedule_task(task);
    scheduler_service.schedule_task(stopper);

    event_loop.loop();
}

// ThreadBridge connects:
//   1. parcel queues
//   2. buffer ownership
//   3. interrupts

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    ServiceRegistry service_registry;
    SystemService system_service{service_registry};
    MemoryService memory_service{service_registry};
    SchedulerService scheduler_service{service_registry};

    run_until_complete<TickTask>(service_registry);

    return 0;
}
