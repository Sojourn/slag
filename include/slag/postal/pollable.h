#pragma once

#include <cassert>
#include "slag/postal/event.h"

namespace slag::postal {

    template<EventType event_type>
    class Pollable;

    using Readable = Pollable<EventType::READABLE>;
    using Writable = Pollable<EventType::WRITABLE>;
    using Runnable = Pollable<EventType::RUNNABLE>;
    using Reapable = Pollable<EventType::REAPABLE>;

    template<>
    class Pollable<EventType::READABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& readable_event() = 0;
    };

    template<>
    class Pollable<EventType::WRITABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& writable_event() = 0;
    };

    template<>
    class Pollable<EventType::RUNNABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& runnable_event() = 0;
    };

    template<>
    class Pollable<EventType::REAPABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& reapable_event() = 0;
    };

    template<EventType event_type>
    inline Event& get_pollable_event(Pollable<event_type>& pollable) {
        if constexpr (event_type == EventType::READABLE) {
            return pollable.readable_event();
        }
        if constexpr (event_type == EventType::WRITABLE) {
            return pollable.writable_event();
        }
        if constexpr (event_type == EventType::RUNNABLE) {
            return pollable.runnable_event();
        }
        if constexpr (event_type == EventType::REAPABLE) {
            return pollable.reapable_event();
        }

        abort(); // internal error
    }

}
