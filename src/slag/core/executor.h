#pragma once

#include "task.h"
#include "event.h"
#include "selector.h"
#include "pollable.h"

namespace slag {

    class Executor final : public Pollable<PollableType::RUNNABLE> {
    public:
        Event& runnable_event() override;

        void schedule(Task& task);
        void run(size_t budget = 32);

    private:
        Selector selector_;
    };

}
