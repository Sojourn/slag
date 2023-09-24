#pragma once

#include <cassert>
#include "slag/core/event.h"

// CANCELED
// REAPABLE
// UNPACKED
// UNLOADED
// DEPLETED
// PRODUCED resource has been produced above a watermark?
// CONSUMED resource has been consumed below a watermark?
#define SLAG_POLLABLE_TYPES(X) \
    X(READABLE)                \
    X(WRITABLE)                \
    X(RUNNABLE)                \
    X(COMPLETE)                \

namespace slag {

    enum class PollableType {
#define X(SLAG_POLLABLE_TYPE) \
        SLAG_POLLABLE_TYPE,

        SLAG_POLLABLE_TYPES(X)
#undef X
    };

    // Wrap classes so that they support being pollable.
    template<typename T>
    class PollableAdapter;

    template<PollableType type>
    class Pollable;

    template<>
    class Pollable<PollableType::READABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& readable_event() = 0;

        bool is_readable() const {
            auto&& self = const_cast<Pollable&>(*this);
            return self.readable_event().is_set();
        }
    };

    template<>
    class Pollable<PollableType::WRITABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& writable_event() = 0;

        bool is_writable() const {
            auto&& self = const_cast<Pollable&>(*this);
            return self.writable_event().is_set();
        }
    };

    template<>
    class Pollable<PollableType::RUNNABLE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& runnable_event() = 0;

        bool is_runnable() const {
            auto&& self = const_cast<Pollable&>(*this);
            return self.runnable_event().is_set();
        }
    };

    template<>
    class Pollable<PollableType::COMPLETE> {
    public:
        virtual ~Pollable() = default;

        virtual Event& complete_event() = 0;

        bool is_complete() const {
            auto&& self = const_cast<Pollable&>(*this);
            return self.complete_event().is_set();
        }
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
