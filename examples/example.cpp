
#include <iostream>
#include <any>
#include <span>
#include <memory>
#include <vector>
#include <functional>
#include "slag/slag.h"

using namespace slag;

struct PrintTask : Task {
    size_t activations = 0;
    Event  event;

    void run() override {
        std::cout << (activations++) << std::endl;

        if (activations >= 10) {
            event.set();
        }
        else {
            schedule();
        }
    }
};

class StopperTask
    : public EventObserver
    , public Task
{
public:
    StopperTask(Event& event, Executor& executor=local_executor())
        : Task{executor}
    {
        wait(event, nullptr);
    }

    void run() override {
        std::cout << "Stopping event loop!" << std::endl;
        local_event_loop().stop();
    }

private:
    void handle_event_set(Event&, void*) override {
        schedule();
    }

    void handle_event_destroyed(void*) override {
        abort();
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    EventLoop event_loop{std::make_unique<IOURingReactor>()};

    PrintTask print_task;
    print_task.schedule();

    StopperTask stopper_task{print_task.event};

    event_loop.run();

    return 0;
}
