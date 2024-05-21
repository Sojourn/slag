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

    class Thread : public mantle::ObjectFinalizer {
    public:
        Thread(Application& application, std::unique_ptr<Task> task);
        virtual ~Thread();

        Thread(Thread&&) = delete;
        Thread(const Thread&) = delete;
        Thread& operator=(Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;

        EventLoop& event_loop();

        void start();

    private:
        void run();

        void finalize(mantle::Object& object) noexcept override;
        void finalize(Buffer& buffer) noexcept;
        void finalize(Operation& operation) noexcept;

    private:
        Application&             application_;
        mantle::ObjectFinalizer* finalizer_;
        EventLoop*               event_loop_;
        std::unique_ptr<Task>    root_task_;
        std::thread              thread_;
    };

}
