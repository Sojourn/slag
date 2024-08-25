#pragma once

#include <memory>
#include "core.h"
#include "system.h"
#include "drivers/region_driver.h"

namespace slag {

    class EventLoop final
        : private Finalizer
        , private InterruptHandler
    {
    public:
        explicit EventLoop(Domain& domain);
        ~EventLoop();

        EventLoop(EventLoop&&) = delete;
        EventLoop(const EventLoop&) = delete;
        EventLoop& operator=(EventLoop&&) = delete;
        EventLoop& operator=(const EventLoop&) = delete;

        bool is_running() const;

        Region& region();
        Reactor& reactor();
        Executor& executor(TaskPriority priority);

        template<typename RootTask, typename... Args>
        void run(Args&&... args);
        void stop(bool force = false);

    private:
        void finalize(ObjectGroup group, std::span<Object*> objects) noexcept override;

        void finalize(Buffer& buffer);
        void finalize(FileDescriptor& file_descriptor);
        void finalize(Operation& operation);

    private:
        void handle_interrupt(Interrupt interrupt) override final;

        void loop();

    private:
        using InterruptVector = std::array<Event, INTERRUPT_REASON_COUNT>;

        Region                        region_;
        Reactor                       reactor_;
        InterruptVector               interrupt_vector_;

        TaskPriority                  current_priority_;
        Executor                      high_priority_executor_;
        Executor                      idle_priority_executor_;

        std::optional<RegionDriver>   region_driver_;
        std::unique_ptr<Task>         root_task_;
    };

    template<typename RootTask, typename... Args>
    void EventLoop::run(Args&&... args) {
        if (root_task_) {
            throw std::runtime_error("Already running");
        }

        // Construction of drivers is deferred until we have a `ThreadContext`.
        region_driver_.emplace(region_, reactor_);

        root_task_ = std::make_unique<RootTask>(std::forward<Args>(args)...);

        loop();
    }

}
