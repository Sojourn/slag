#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "slag/core/service.h"

namespace slag {

    class Task;
    class OperationBase;

    template<ServiceType type>
    class ServiceInterface;

    template<>
    class ServiceInterface<ServiceType::SYSTEM> : public Service {
    public:
        ServiceInterface(): Service(ServiceType::SYSTEM) {}

        virtual void poll(bool non_blocking) = 0;

    private:
        friend class OperationBase;

        virtual void handle_operation_abandoned(OperationBase& operation_base) = 0;
        virtual void handle_operation_daemonized(OperationBase& operation_base) = 0;
    };

    // Handles memory allocation, preallocation.
    template<>
    class ServiceInterface<ServiceType::MEMORY> : public Service {
    public:
        ServiceInterface(): Service(ServiceType::MEMORY) {}

        virtual std::span<std::byte> allocate_block() = 0;
        virtual void deallocate_block(std::span<std::byte> block) = 0;
        // virtual std::span<std::byte> allocate_buddy(BuddyCategory category) = 0;
    };

    template<>
    class ServiceInterface<ServiceType::SCHEDULER> : public Service {
    public:
        ServiceInterface()
            : Service(ServiceType::SCHEDULER)
        {
            enter_executor(executor_);
        }

        virtual ~ServiceInterface() {
            leave_executor(executor_);
        }

        virtual void schedule(Task& task) = 0;
        virtual void cancel(Task& task) = 0;

    protected:
        // TODO: Think about unifying resource limiters and executors into a 'Scheduler'.
        // TODO: Think about pass-through resource limits (might only want to constrain a specific resource at a given level).
        friend class Executor;
        friend class ExecutorActivation;

        inline Executor& executor() {
            return *executor_stack_.back();
        }

        inline void enter_executor(Executor& executor) {
            executor_stack_.push_back(&executor);
        }

        inline void leave_executor(Executor& executor) {
            assert(!executor_stack_.empty());
            assert(executor_stack_.back() == &executor);

            executor_stack_.pop_back();
        }

    private:
        Executor               executor_;
        std::vector<Executor*> executor_stack_;
    };

}
