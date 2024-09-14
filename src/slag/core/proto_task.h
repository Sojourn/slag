#pragma once

#include "slag/core/task.h"
#include "slag/core/event.h"
#include "slag/core/pollable.h"

#define SLAG_PT_BEGIN()  \
    switch (pt_state_) { \
        case 0:

#define SLAG_PT_END() }  \
    pt_state_ = 0;       \
    if (!is_failure()) { \
        set_success();   \
    }                    \
    return;              \

#define SLAG_PT_YIELD()                         \
    [[fallthrough]]; case __LINE__:             \
    if (pt_yield_) {                            \
        pt_yield_ = false;                      \
    }                                           \
    else {                                      \
        pt_yield_ = true;                       \
        pt_state_ = __LINE__;                   \
        pt_runnable_ = &Task::runnable_event(); \
        return;                                 \
    }                                           \

#define SLAG_PT_WAIT_EVENT(event)   \
    pt_state_ = __LINE__;           \
    [[fallthrough]]; case __LINE__: \
    if (!event.is_set()) {          \
        pt_runnable_ = &event;      \
        return;                     \
    }                               \

#define SLAG_PT_WAIT_POLLABLE(pollable, pollable_type)                                 \
    pt_state_ = __LINE__;                                                              \
    [[fallthrough]]; case __LINE__:                                                    \
    if (Event& event = get_pollable_event<pollable_type>(pollable); !event.is_set()) { \
        pt_runnable_ = &event;                                                         \
        return;                                                                        \
    }                                                                                  \

#define SLAG_PT_WAIT_READABLE(pollable) SLAG_PT_WAIT_POLLABLE(pollable, PollableType::READABLE)
#define SLAG_PT_WAIT_WRITABLE(pollable) SLAG_PT_WAIT_POLLABLE(pollable, PollableType::WRITABLE)
#define SLAG_PT_WAIT_RUNNABLE(pollable) SLAG_PT_WAIT_POLLABLE(pollable, PollableType::RUNNABLE)
#define SLAG_PT_WAIT_COMPLETE(pollable) SLAG_PT_WAIT_POLLABLE(pollable, PollableType::COMPLETE)

namespace slag {

    // An adaptation of protothreads for our readiness model. To use this,
    // derive it and override the Task::run function. Task::run should use the BEGIN/END
    // macros to setup/cleanup. Use the wait macros inside of them to block until
    // some resource is ready.
    //
    // References:
    //   https://dunkels.com/adam/pt/index.html
    //
    class ProtoTask : public Task {
    public:
        explicit ProtoTask(TaskPriority priority = TaskPriority::SAME)
            : Task{priority}
            , pt_state_{0}
            , pt_yield_{false}
            , pt_runnable_{nullptr}
        {
        }

        explicit ProtoTask(Executor& executor)
            : Task{executor}
            , pt_state_(0)
            , pt_yield_{false}
            , pt_runnable_{nullptr}
        {
        }

        Event& runnable_event() override final {
            return *pt_runnable_;
        }

    protected:
        int    pt_state_;
        bool   pt_yield_;
        Event* pt_runnable_;
    };

}
