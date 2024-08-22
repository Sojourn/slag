#pragma once

#include <memory>
#include <thread>
#include "types.h"
#include "core.h"
#include "mantle/mantle.h"
#include "event_loop.h"
#include "memory/buffer.h"
#include "system/operation.h"

namespace slag {

    class Application;

    class Thread {
    public:
        Thread(Application& application, std::unique_ptr<Task> init);
        ~Thread();

        Thread(Thread&&) = delete;
        Thread(const Thread&) = delete;
        Thread& operator=(Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;

        Application& application();
        EventLoop& event_loop();

        void start();

    private:
        void run();

    private:
        Application&             application_;
        EventLoop*               event_loop_;
        std::unique_ptr<Task>    init_;
        std::thread              thread_;
    };

}
