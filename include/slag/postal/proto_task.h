#pragma once

#include "slag/postal/task.h"
#include "slag/postal/event.h"
#include "slag/postal/pollable.h"

#define SLAG_PT_BEGIN()  \
    switch (pt_state_) { \
        case 0:

#define SLAG_PT_END() }  \
    pt_state_ = 0;       \
    if (!is_failure()) { \
        set_success();   \
    }                    \
    return;              \

#define SLAG_PT_WAIT(resource, pollable_type)                                          \
    pt_state_ = __LINE__;                                                              \
    [[fallthrough]]; case __LINE__:                                                    \
    if (Event& event = get_pollable_event<pollable_type>(resource); !event.is_set()) { \
        pt_runnable_ = &event;                                                         \
        return;                                                                        \
    }                                                                                  \

#define SLAG_PT_WAIT_READABLE(resource) SLAG_PT_WAIT(resource, PollableType::READABLE)
#define SLAG_PT_WAIT_WRITABLE(resource) SLAG_PT_WAIT(resource, PollableType::WRITABLE)
#define SLAG_PT_WAIT_RUNNABLE(resource) SLAG_PT_WAIT(resource, PollableType::RUNNABLE)
#define SLAG_PT_WAIT_COMPLETE(resource) SLAG_PT_WAIT(resource, PollableType::COMPLETE)

namespace slag::postal {

    // An adaptation of protothreads for our readiness model. To use this,
    // derive it and override the Task::run function. Task::run should use the BEGIN/END
    // macros to setup/cleanup. Use the wait macros inside of them to block until
    // some resource is ready.
    //
    // https://dunkels.com/adam/pt/index.html
    //
    // TODO: think naming this ProtoThread vs. ProtoTask. ProtoThread is accurate to the concept,
    //       but ProtoTask is internally consistent.
    //
    class ProtoTask : public Task {
    public:
        ProtoTask()
            : pt_state_{0}
            , pt_event_{}
            , pt_runnable_{&pt_event_}
        {
            pt_event_.set();
        }

        Event& runnable_event() override final {
            return *pt_runnable_;
        }

    protected:
        int    pt_state_;
        Event  pt_event_;
        Event* pt_runnable_;
    };

}
