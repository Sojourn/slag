#pragma once

#include <memory>
#include <vector>
#include "slag/layer.h"
#include "slag/core/selector.h"
#include "slag/core/executor.h"
#include "slag/postal/service.h"

namespace slag {

    template<typename Stack>
    class WorkerService
        : public Service
        , public Layer<WorkerService, Stack>
    {
    public:
        using Base = Layer<WorkerService, Stack>;

        using Base::above;
        using Base::below;

    public:
        WorkerService();

        // Create and schedule a new task.
        template<typename T, typename... Args>
        void spawn(Args&&... args);

        void start();
        void stop();

    public:
        Event& runnable_event() override final;

        void run() override final;

    private:
        void reap_task(Task& task);

    private:
        Executor                           executor_;
        Selector                           selector_;
        std::vector<std::unique_ptr<Task>> tasks_;
    };

}

#include "worker_service.hpp"
