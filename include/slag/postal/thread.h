#pragma once

#include <thread>
#include <future>
#include "slag/postal/config.h"
#include "slag/postal/domain.h"

namespace slag::postal {

    template<typename Driver>
    class Thread {
        Thread(Thread&&) = delete;
        Thread(const Thread&) = delete;
        Thread& operator=(Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;

        static_assert(std::is_base_of_v<Task, Driver>, "Driver must be derived from Task.");

    public:
        template<typename... DriverArgs>
        Thread(const RegionConfig& config, DriverArgs&&... driver_args);
        ~Thread();

        std::future<void> get_future();

    private:
        void run(Driver& driver);

    private:
        std::thread        thread_;
        std::promise<void> complete_promise_;
    };

}

#include "thread.hpp"
