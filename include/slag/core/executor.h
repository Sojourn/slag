#pragma once

#include <chrono>
#include "slag/core/task.h"
#include "slag/core/event.h"
#include "slag/core/selector.h"
#include "slag/core/pollable.h"

namespace slag {

    class Executor : public Task {
    public:
        using Quantum = std::chrono::microseconds;

        explicit Executor(const Quantum& quantum = std::chrono::microseconds(10));

        // TODO: extract this into a Scheduler/Interrupt
        const Quantum& quantum() const;
        void set_quantum(const Quantum& quantum);

        void insert(Task& task);
        void remove(Task& task);

        // Returns an event that is set when we have runnable tasks.
        Event& runnable_event() override;

        // Runs tasks until the quantum has elapsed, or there are
        // no more runnable tasks.
        void run() override;

    private:
        using Deadline = std::chrono::time_point<std::chrono::steady_clock>;

        void run_until(Task& task, const Deadline& deadline);

    private:
        Quantum  quantum_;
        Selector selector_;
    };

}
