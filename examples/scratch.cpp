#include <iostream>
#include "slag/slag.h"

using namespace slag;

Coroutine<int> bar(int value) {
    co_return value;
}

Coroutine<Void> foo() {
    std::cout << "Running!" << std::endl;

    std::cout << (co_await bar(14)) << std::endl;
    std::cout << (co_await bar(17)) << std::endl;

    local_event_loop().stop();

    co_return Void{};
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    EventLoop event_loop{std::make_unique<IOURingReactor>()};

    Fiber<Void> fiber{foo};

    event_loop.run();

    return 0;
}
