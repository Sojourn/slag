#pragma once

#include <cstdint>
#include <cstddef>
#include "slag/intrusive_queue.h"

// Rename these to PollableType?
#define SLAG_EVENT_TYPES(X) \
    X(READABLE)             \
    X(WRITABLE)             \
    X(RUNNABLE)             \
    X(COMPLETE)             \
    X(CANCELED)             \
    X(REAPABLE)             \

namespace slag::postal {

    class Selector;

    enum class EventType : uint8_t {
#define X(SLAG_EVENT_TYPE) \
        SLAG_EVENT_TYPE,   \

        SLAG_EVENT_TYPES(X)
#undef X
    };

    class Event {
    public:
        Event();
        Event(Event&& other);
        Event(const Event&) = delete;

        Event& operator=(Event&& other);
        Event& operator=(const Event&) = delete;

        bool is_set() const;
        void set(bool value = true);
        void reset();

        void* user_data();
        const void* user_data() const;
        void set_user_data(void* user_data);

    private:    
        friend class Selector;

        void attach(Selector& selector);
        void detach(Selector& selector);

    private:
        void*              user_data_;
        Selector*          selector_;
        IntrusiveQueueNode selector_hook_;
        bool               is_set_;
    };

}
