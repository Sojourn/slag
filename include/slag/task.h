#pragma once

#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <slag/event.h>

namespace slag {

    class TaskGroup;

    enum class TaskState {
        WAITING,
        PENDING,
        RUNNING,
        COMPLETE,
    };

    class CoroutineWrapper {
    public:
        CoroutineWrapper() = default;
        CoroutineWrapper(CoroutineWrapper&&) noexcept = delete;
        CoroutineWrapper(const CoroutineWrapper&) = delete;
        virtual ~CoroutineWrapper() = default;

        CoroutineWrapper& operator=(CoroutineWrapper&&) noexcept = delete;
        CoroutineWrapper& operator=(const CoroutineWrapper&) = delete;

        virtual void run(Task& task);
    };

    class Task : public EventObserver {
    public:
        template<typename Coroutine>
        Task(TaskGroup& group, Coroutine&& coroutine);
        Task(Task&&) noexcept = delete;
        Task(const Task&) = delete;
        virtual ~Task();

        Task& operator=(Task&&) noexcept = delete;
        Task& operator=(const Task&) = delete;

        [[nodiscard]] TaskGroup& group();
        [[nodiscard]] const TaskGroup& group() const;
        [[nodiscard]] TaskState state() const;
        [[nodiscard]] Event& on_state_transition();
        [[nodiscard]] const Event& on_state_transition() const;

    private:
        void set_state(TaskState state);

    private:
        static constexpr size_t COROUTINE_STORAGE_SIZE = 128;

        class CoroutineWrapper;
        using CoroutineStorage = std::aligned_storage_t<
            COROUTINE_STORAGE_SIZE,
            alignof(std::max_align_t)
        >;

        TaskGroup&       group_;
        TaskState        state_;
        CoroutineStorage storage_;
        Event            on_state_transition_;
    };

}
