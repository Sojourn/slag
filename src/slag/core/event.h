#pragma once

#include <cstdint>
#include <cstddef>
#include "slag/collections/intrusive_queue.h"

namespace slag {

    class Selector;

    class Event {
    public:
        Event();
        Event(Event&& other);
        Event(const Event&) = delete;

        Event& operator=(Event&& other);
        Event& operator=(const Event&) = delete;

        explicit operator bool() const;

        bool is_set() const;
        void set(bool value = true);
        void reset();

        void* user_data();
        const void* user_data() const;
        void set_user_data(void* user_data);

        template<typename T>
        T& cast_user_data();

        template<typename T>
        const T& cast_user_data() const;

        bool is_linked() const;
        void unlink();

    private:    
        friend class Selector;

        void attach(Selector& selector);
        void detach(Selector& selector);

    private:
        void*              user_data_;
        Selector*          selector_;
        IntrusiveQueueNode selector_hook_;

        // TODO: Use a tagged pointer to compact this object.
        bool               is_set_;
    };

    template<typename T>
    T& Event::cast_user_data() {
        return *static_cast<T*>(user_data_);
    }

    template<typename T>
    const T& Event::cast_user_data() const {
        return *static_cast<const T*>(user_data_);
    }

}
