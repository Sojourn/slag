#pragma once

#include "slag/core.h"
#include "slag/resource.h"
#include "slag/bus.h"
#include "slag/system.h"
#include "slag/driver.h"

#include <memory>
#include <optional>
#include <stdexcept>

namespace slag {

    class EventLoop final : private Finalizer {
    public:
        EventLoop(Domain& domain, std::shared_ptr<Fabric> fabric, std::shared_ptr<Reactor> reactor);
        ~EventLoop() = default;

        EventLoop(EventLoop&&) = delete;
        EventLoop(const EventLoop&) = delete;
        EventLoop& operator=(EventLoop&&) = delete;
        EventLoop& operator=(const EventLoop&) = delete;

        bool is_running() const;

        Region& region();
        Router& router();
        Reactor& reactor();
        Executor& executor(TaskPriority priority);

        template<typename RootTask, typename... Args>
        void run(Args&&... args);
        void run(std::unique_ptr<Task> root_task);

        void stop(bool force = false);

    private:
        void finalize(ObjectGroup group, std::span<Object*> objects) noexcept override;

        void finalize(Managed& managed);
        void finalize(Message& message);
        void finalize(Buffer& buffer);
        void finalize(FileDescriptor& file_descriptor);
        void finalize(Operation& operation);

    private:
        void loop();

    private:
        Region                        region_;
        Router                        router_;
        std::shared_ptr<Reactor>      reactor_;
        InterruptVector               interrupt_vector_;

        TaskPriority                  current_priority_;
        Executor                      high_priority_executor_;
        Executor                      idle_priority_executor_;

        std::optional<Drivers>        drivers_;
        std::unique_ptr<Task>         root_task_;
    };

    template<typename RootTask, typename... Args>
    void EventLoop::run(Args&&... args) {
        if (root_task_) {
            throw std::runtime_error("Already running");
        }

        run(std::make_unique<RootTask>(std::forward<Args>(args)...));
    }

}
