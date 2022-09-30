#include <iostream>
#include <any>
#include <span>
#include <memory>
#include <vector>
#include <functional>
#include <coroutine>
#include <variant>
#include <type_traits>
#include <stdexcept>
#include "slag/slag.h"

using namespace slag;

Coroutine<int> foo(Future<int> x) {
    info("awaiting");
    co_await x;
    info("await complete");
    co_return 3;
}

struct Stopper : public EventObserver {
    void handle_event_set(Event& event, void* user_data) {
        (void)event;
        (void)user_data;

        info("Stopper::handle_event_set");
        local_event_loop().stop();
    }

    void handle_event_destroyed(void* user_data) {
        (void)user_data;

        info("Stopper::handle_event_destroyed");
        local_event_loop().stop();
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    EventLoop event_loop{std::make_unique<IOURingReactor>()};

    Promise<int> promise;
    promise.set_value(14);

    Fiber<int> fiber{foo, promise.get_future()};
    Future<int> fiber_future = fiber.get_future();

    Stopper stopper;
    stopper.wait(fiber_future.event(), nullptr);

    event_loop.run();

    return 0;
}
