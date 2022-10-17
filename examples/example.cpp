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
    info("run-server enter");

    Socket socket;

    // initialize the socket
    {
        info("pre-result");
        auto result = co_await socket.open(AF_INET, SOCK_STREAM, 0);
        info("post-result");
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

    (void)address;

    info("run-server exit");
    co_return 0;
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

    Fiber<int> server_fiber{run_server, Address{}};

    Stopper stopper;
    stopper.wait(server_fiber.completion(), nullptr);

    event_loop.run();

    return 0;
}
