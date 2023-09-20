#pragma once

#include "slag/stack.h"
#include "slag/postal/task.h"
#include "slag/postal/executor.h"

namespace slag::postal {

    template<template<typename> class... Services>
    class Driver {
    public:
        Driver();

        void run();

        // Gracefully shutdown. This can be called once 
        void stop();

        // Shutdown immediately without stopping services. This can be called while
        // already stopping in case that is taking too long.
        void halt();

    private:
        enum class State {
            INITIAL,
            RUNNING,
            STOPPING,
            HALTING,
            STOPPED,
        };

        constexpr static bool is_valid_transition(State old_state, State new_state);

        void set_state(State state);
        void start_services();
        void stop_services();
        void step();

    private:
        State              state_;
        Executor           executor_;
        Stack<Services...> service_stack_;
    };

}

#include "driver.hpp"
