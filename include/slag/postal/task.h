#pragma once

#include "slag/postal/event.h"
#include "slag/postal/pollable.h"

namespace slag::postal {

    class Task : public Runnable {
    public:
        // This should execute a small, but meaningful amount of work. It will
        // be periodically called by an executor when the task indicates
        // that it is runnable (the runnable event is set).
        virtual void run() = 0;
    };

}
