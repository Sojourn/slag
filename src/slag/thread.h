#pragma once

#include <memory>
#include <thread>
#include "types.h"
#include "core.h"
#include "system.h"
#include "event_loop.h"

namespace slag {

    class Application;

    class Thread {
        Thread(Thread&&) = delete;
        Thread(const Thread&) = delete;
        Thread& operator=(Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;

    public:
        Thread(Application& application, std::unique_ptr<Task> task);
        ~Thread();

        ThreadIndex index() const;

        void start();

    private:
        friend class Context;

        ResourceTables& resource_tables();
        EventLoop& event_loop();

    private:
        void run();

    private:
        struct Components {
            ResourceTables* resource_tables;
            EventLoop*      event_loop;
        };

        Application&          application_;
        Components*           components_;
        std::thread           thread_;
        ThreadIndex           index_;
        std::unique_ptr<Task> task_;
    };

}
