
#include <iostream>
#include <any>
#include <span>
#include <memory>
#include <vector>
#include <functional>
#include "slag/slag.h"

using namespace slag;

struct MyTask : Task {
    size_t activations = 0;

    void run() override {
        std::cout << (activations++) << std::endl;

        if (activations >= 10) {
            local_event_loop().stop();
        }
        else {
            schedule();
        }
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    EventLoop event_loop{std::make_unique<IOURingReactor>()};

    MyTask task;
    task.schedule();

    event_loop.run();

    return 0;
}
