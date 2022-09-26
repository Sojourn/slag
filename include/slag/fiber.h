#pragma once

#include <coroutine>
#include "slag/task.h"
#include "slag/event.h"

namespace slag {

    class Fiber
        : private Task
        , private EventObserver
    {
    public:
        template<typename InitFunctor, typename... Args>
        Fiber(InitFunctor&& init_functor, Args&&... args);

    private:
        void run() override;
        void handle_event_set(Event& event, void* user_data) override;
        void handle_event_destroyed(void* user_data) override;

    private:
        // TODO: stack of type-erased coroutine handles
        // TODO: linear allocator for coroutine states
    };

}
