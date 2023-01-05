#include <iostream>
#include "slag/slag.h"

using namespace slag;

Future<void> make_ready_future() {
    auto&& [future, promise] = make_future<void>();
    asm("int $3");
    promise.set_default_value();
    return std::move(future);
}

Coroutine<int> bar(int value) {
    co_return value;
}

Coroutine<Void> foo() {
    std::cout << "Running!" << std::endl;

    co_await make_ready_future();

    std::cout << (co_await bar(14)) << std::endl;
    std::cout << (co_await bar(17)) << std::endl;

    local_event_loop().stop();

    co_return Void{};
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Future<void> future = make_ready_future();
    (void)future.get();

    // EventLoop event_loop{std::make_unique<IOURingReactor>()};

    // Fiber<Void> fiber{foo};

    // event_loop.run();

    return 0;
}
