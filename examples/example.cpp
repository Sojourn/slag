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

    // initialize the listening socket
    {
        {
            auto result = co_await socket.open(address.family(), SOCK_STREAM, 0);
            assert(result.has_value());
        }

        {
            auto result = co_await socket.bind(address);
            assert(result.has_value());
        }

        {
            auto result = co_await socket.listen();
            assert(result.has_value());
        }
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

    std::cout << strerror(99) << std::endl;

    AddressQuery address_query;
    address_query.host_name = "localhost";
    address_query.family    = AF_INET;
    address_query.type      = SOCK_STREAM;
    address_query.passive   = true;

    Address address;
#if 0
    try {
        std::vector<Address> addresses = execute(address_query);
        assert(!addresses.empty());
        address = addresses.front();
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
#else
    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family      = AF_INET;
    addr_in.sin_port        = htons(10334);
    addr_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    address = Address{addr_in};
#endif

    Fiber<int> server_fiber{run_server, address};

    Stopper stopper;
    stopper.wait(server_fiber.completion(), nullptr);

    event_loop.run();

    return 0;
}
