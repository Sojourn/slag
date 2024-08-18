#pragma once

#include <chrono>
#include "task.h"
#include "event.h"
#include "selector.h"
#include "pollable.h"

namespace slag {

    // TODO: Remove deadlines to make this deterministic.
    class Executor : public Task {
    public:
        Event& runnable_event() override;

        void schedule(Task& task);
        void run() override;

    private:
        using Deadline = std::chrono::time_point<std::chrono::steady_clock>;

        void run_until(Task& task, const Deadline& deadline);

    private:
        Selector scheduler_;
    };

}
