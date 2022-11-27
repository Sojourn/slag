#pragma once

#include <coroutine>
#include <vector>
#include <cstddef>
#include "slag/task.h"
#include "slag/event.h"

namespace slag {

    class EventLoop;

    class FiberBase : public Task {
    public:
        FiberBase() = default;
        FiberBase(FiberBase&&) = delete;
        FiberBase(const FiberBase&) = delete;
        virtual ~FiberBase() = default;

        FiberBase& operator=(FiberBase&&) = delete;
        FiberBase& operator=(const FiberBase&) = delete;

        [[nodiscard]] bool is_done() const;
        [[nodiscard]] virtual Event& completion() = 0;
        [[nodiscard]] virtual const Event& completion() const = 0;

    protected:
        friend class Awaitable;
        friend class AwaitableBase;

        void resume(std::coroutine_handle<> handle, TaskPriority priority=TaskPriority::NORMAL);

        void run() override;

        // TODO: make operator new on Coroutines use this
        // TODO: support a segmented stack
        //
        // void* allocate_stack_frame(size_t size);
        // void deallocate_stack_frame(size_t size);

    protected:
        class [[nodiscard]] Activation {
        public:
            Activation();
            Activation(FiberBase& fiber);
            Activation(Activation&&) = delete;
            Activation(const Activation&) = delete;
            ~Activation();

            Activation& operator=(Activation&&) = delete;
            Activation& operator=(const Activation&) = delete;

        private:
            EventLoop& event_loop_;
            FiberBase* previous_active_fiber_;
        };

    private:
        std::coroutine_handle<> pending_handle_;
    };

    [[nodiscard]] FiberBase* local_active_fiber();

}
