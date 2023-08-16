#pragma once

#include <cassert>
#include "slag/postal/event.h"

// CANCELED?
// REAPABLE?
#define SLAG_POLLABLE_TYPES(X) \
    X(READABLE)                \
    X(WRITABLE)                \
    X(RUNNABLE)                \
    X(COMPLETE)                \

namespace slag::postal {

    enum class PollableType {
#define X(SLAG_POLLABLE_TYPE) \
        SLAG_POLLABLE_TYPE,

        SLAG_POLLABLE_TYPES(X)
#undef X
    };

    template<PollableType type>
    class Pollable;

    template<>
    class Pollable<PollableType::READABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& readable_event() = 0;
    };

    template<>
    class Pollable<PollableType::WRITABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& writable_event() = 0;
    };

    template<>
    class Pollable<PollableType::RUNNABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& runnable_event() = 0;
    };

    template<>
    class Pollable<PollableType::COMPLETE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& complete_event() = 0;
    };

    template<PollableType type>
    inline Event& get_pollable_event(Pollable<type>& pollable) {
        if constexpr (type == PollableType::READABLE) {
            return pollable.readable_event();
        }
        if constexpr (type == PollableType::WRITABLE) {
            return pollable.writable_event();
        }
        if constexpr (type == PollableType::RUNNABLE) {
            return pollable.runnable_event();
        }
        if constexpr (type == PollableType::COMPLETE) {
            return pollable.complete_event();
        }

        abort(); // internal error
    }

}
