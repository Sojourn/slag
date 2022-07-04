#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

class TestResource : public Resource {
public:
    using Resource::start_nop_operation;

    void handle_operation_complete(Operation&) override {
    }
};

TEST_CASE("io_uring reactor startup and shutdown", "[IOURingReactor]") {
    EventLoop event_loop{std::make_unique<IOURingReactor>()};

    SECTION("cancel working") {
        TestResource resource;
        (void)resource.start_nop_operation(nullptr);
    }
}
