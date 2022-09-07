#pragma once

#include <type_traits>
#include <cstddef>
#include <cstdint>
#include "slag/event.h"
#include "slag/error.h"

namespace slag {

    enum class TaskState {
        WAITING,
        PENDING,
        RUNNING,
        COMPLETE,
    };

    class Task : public EventObserver {
    public:
        Task() = default;
        Task(Task&&) noexcept = delete;
        Task(const Task&) = delete;
        virtual ~Task();

        Task& operator=(Task&&) noexcept = delete;
        Task& operator=(const Task&) = delete;

        [[nodiscard]] TaskState state() const;
        [[nodiscard]] Error error() const;
        [[nodiscard]] TaskState run();

    // private:
        void set_state(TaskState state);
        virtual void set_error(Error error);

        void handle_event_set(Event& event, void* user_data) override;
        void handle_event_destroyed(void* user_data) override;

        virtual void run_impl() = 0;

    private:
        TaskState state_;
        Error     error_;
    };

}
