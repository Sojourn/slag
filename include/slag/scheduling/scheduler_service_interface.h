#pragma once

#include <algorithm>
#include <cassert>
#include "slag/core/task.h"
#include "slag/core/selector.h"
#include "slag/core/executor.h"
#include "slag/core/service.h"
#include "slag/core/service_interface.h"
#include "slag/system/operation_base.h"

namespace slag {

    enum class ExecutorStack {
        BUSY, // Busy tasks get priority to run.
        IDLE, // Idle tasks only run when there are no busy tasks.
        SAME, // Same stack as our parent task.
    };

    template<>
    class ServiceInterface<ServiceType::SCHEDULER> : public Service {
    public:
        explicit ServiceInterface(ServiceRegistry& service_registry)
            : Service{ServiceType::SCHEDULER, service_registry}
        {
        }

        ~ServiceInterface() {
            assert(executor_stack_.empty());
        }

        void start_service() override {
            assert(executor_stack_.empty());
        }

        void stop_service() override {
            assert(executor_stack_.empty());
        }

        virtual void schedule_task(Task& task) = 0;
        virtual bool is_quiescent() const = 0;
        virtual void run() = 0;

    protected:
        friend class Executor;
        friend class ExecutorActivation;

        Executor& active_executor() {
            if (executor_stack_.empty()) {
                throw std::runtime_error("No running executor");
            }

            return *executor_stack_.back();
        }

        void enter_executor(Executor& executor) {
            assert(std::count(executor_stack_.begin(), executor_stack_.end(), &executor) == 0);

            executor_stack_.push_back(&executor);
        }

        void leave_executor(Executor& executor) {
            assert(!executor_stack_.empty());
            assert(executor_stack_.back() == &executor);

            executor_stack_.pop_back();
        }

    private:
        std::vector<Executor*> executor_stack_;
    };

}
