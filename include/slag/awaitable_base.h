#pragma once

#include <coroutine>
#include "slag/event.h"

namespace slag {

    class AwaitableBase : public EventObserver {
    public:
        AwaitableBase(Event& event);

        [[nodiscard]] bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<> handle);

    private:
        void handle_event_set(Event& event, void* user_data) override;
        void handle_event_destroyed(void* user_data) override;

    private:
        Event&                  event_;
        std::coroutine_handle<> handle_;
    };

}
