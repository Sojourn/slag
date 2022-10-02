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

Coroutine<int> run_server(const Address& address) {
    (void)address;

    Socket socket;

    // initialize the socket
    {
        Result<void> result;

        result = co_await socket.open(address.family(), SOCK_STREAM, 0);
        assert(result.has_value());

        // result = co_await socket.bind(address);
        // assert(result.has_value());

        // result = co_await socket.listen();
        // assert(result.has_value());
    }

    // while (Result<Socket> connection_result = co_await socket.accept()) {
    //     if (connection_result.has_value()) {
    //         // TODO: create a new fiber to handle this connection
    //         (void)std::move(connection_result.value());
    //     }
    //     else {
    //         break;
    //     }
    // }

    co_return 0;
}

Coroutine<int> foo() {
    co_return co_await run_server(Address{});
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

    Fiber<int> server_fiber{run_server, Address{}};

    Stopper stopper;
    Future<int> stop_future = server_fiber.get_future();
    stopper.wait(stop_future.event(), nullptr);

    event_loop.run();

    return 0;
}
