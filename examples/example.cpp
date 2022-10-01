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
    info("awaiting future");
    co_await x;
    info("await future complete");
    co_return 3;
}

Coroutine<int> bar() {
    co_return 7;
}

Coroutine<int> baz(Fiber<int>& fiber) {
    info("awaiting fiber");
    int fiber_result = co_await fiber;
    info("await fiber complete");

    co_return 1 + fiber_result;
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

    Fiber<int> fiber1{foo, promise.get_future()};
    Fiber<int> fiber2{bar};
    Fiber<int> fiber3{baz, fiber2};

    Stopper stopper;
    Future<int> stop_future = fiber3.get_future();
    stopper.wait(stop_future.event(), nullptr);

    event_loop.run();

    return 0;
}
