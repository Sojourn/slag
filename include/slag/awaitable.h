#pragma once

#include <coroutine>
#include <initializer_list>
#include "slag/pollable.h"
#include "slag/fiber_base.h"

namespace slag {

    class Awaitable : public Pollable::Observer {
    public:
        Awaitable(Pollable& pollable, PollableEventMask events);

        bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<> handle);

        void handle_pollable_event(Pollable& pollable, Pollable::Event event) override;
        void handle_pollable_destroyed(Pollable& pollable) override;

    private:
        Pollable&               pollable_;
        PollableEventMask       events_;
        FiberBase*              fiber_;
        std::coroutine_handle<> handle_;
    };

}
