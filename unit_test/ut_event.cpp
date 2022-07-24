#include "catch.hpp"
#include "slag/slag.h"
#include <cstdint>
#include <cstddef>
#include <cassert>

using namespace slag;

struct TestEventObserver : public EventObserver {
    size_t event_set_count = 0;
    size_t event_destroyed_count = 0;

    void handle_event_set(Event& event, void* user_data) {
        (void)event;
        (void)user_data;
        ++event_set_count;
    }

    void handle_event_destroyed(void* user_data) {
        (void)user_data;
        ++event_destroyed_count;
    }
};

TEST_CASE("event lifetime and notification", "[Event]") {
    TestEventObserver event_observer;

    SECTION("destroyed") {
        {
            Event event;
            event_observer.wait(event, nullptr);
        }

        CHECK(event_observer.event_set_count == 0);
        CHECK(event_observer.event_destroyed_count == 1);
    }

    SECTION("set and destroyed") {
        {
            Event event;
            event_observer.wait(event, nullptr);

            event.set();
        }

        CHECK(event_observer.event_set_count == 1);
        CHECK(event_observer.event_destroyed_count == 1);
    }
}
