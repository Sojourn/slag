#pragma once

#include <chrono>
#include "slag/postal/task.h"
#include "slag/postal/event.h"
#include "slag/postal/selector.h"
#include "slag/postal/pollable.h"

namespace slag::postal {

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

        // Runs the next runnable task.
        void run() override;

    private:
        Quantum  quantum_;
        Selector selector_;
    };

}
