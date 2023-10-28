#pragma once

#include <cstddef>
#include <cstdint>
#include "slag/core/service.h"
#include "slag/core/service_interface.h"
#include "slag/core/service_registry.h"
#include "slag/core/executor.h"

namespace slag {

    class ServiceRegistry;

    class EventLoop {
        EventLoop(EventLoop&&) = delete;
        EventLoop(const EventLoop&) = delete;
        EventLoop& operator=(EventLoop&&) = delete;
        EventLoop& operator=(const EventLoop&) = delete;

    public:
        explicit EventLoop(ServiceRegistry& service_registry);
        ~EventLoop();

        ServiceRegistry& service_registry();
        const ServiceRegistry& service_registry() const;

    public:
        // Control whether the event loop is allowed to block while stepping.
        // An EventLoop that is spinning will never block.
        bool is_spining() const;
        void set_spinning();
        void reset_spinning();

        // Run until a stop is requested.
        void loop();

        // Advance the event loop one step. Returns true until the loop stops.
        bool step();

        // Request that the event loop stop soon.
        void stop();

    private:
        friend class EventLoopActivation;

        bool is_stepping() const;
        void set_stepping();
        void reset_stepping();

    private:
        // Check that invariants which could prevent us from blocking are satisfied.
        bool can_block() const;

        // Check invariants, and that heuristics indicate that spinning wouldn't elide context switches.
        bool should_block() const;

        // Executes a step without guards or checks. This allows us to amortize these while looping.
        void step_unsafe();

    private:
        // TODO: Make this be duration (scaled time-stamp-counter delta). The value
        //       should probably be a dynamic number of microseconds based a
        //       operation completion time distribution. Could use a
        //       feedback loop to tune / be hardware agnostic.
        static constexpr uint32_t MAX_IDLE_STREAK = 16;

        ServiceRegistry& service_registry_;
        bool             spinning_;
        bool             stepping_;
        bool             stopping_;
        uint32_t         idle_streak_;
    };

    EventLoop& get_event_loop();

}
