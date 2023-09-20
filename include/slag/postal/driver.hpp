#include <cassert>

namespace slag::postal {

    template<template<typename> class... Services>
    Driver<Services...>::Driver()
        : state_{State::INITIAL}
    {
    }

    template<template<typename> class... Services>
    void Driver<Services...>::run() {
        if (state_ != State::INITIAL) {
            assert(false);
            throw std::runtime_error("Driver has already been run");
        }

        start_services();
        {
            // Run until someone calls stops/halts us.
            while (state_ == State::RUNNING) {
                step();
            }
        }
        stop_services();
    }

    template<template<typename> class... Services>
    void Driver<Services...>::stop() {
        set_state(State::STOPPING);
    }

    template<template<typename> class... Services>
    void Driver<Services...>::halt() {
        set_state(State::HALTING);
    }

    template<template<typename> class... Services>
    constexpr bool Driver<Services...>::is_valid_transition(State old_state, State new_state) {
        switch (old_state) {
            case State::INITIAL: {
                return new_state == State::RUNNING;
            }
            case State::RUNNING: {
                return new_state == State::STOPPING || new_state == State::HALTING;
            }
            case State::STOPPING: {
                return new_state == State::STOPPING || new_state == State::HALTING || new_state == State::STOPPED;
            }
            case State::HALTING: {
                return new_state == State::HALTING || new_state == State::STOPPED;
            }
            case State::STOPPED: {
                return false; // Terminal state.
            }
        }

        abort(); // Unreachable.
    }

    template<template<typename> class... Services>
    void Driver<Services...>::set_state(State state) {
        if (!is_valid_transition(state_, state)) {
            throw std::runtime_error("Invalid state transition");
        }

        state_ = state;
    }

    template<template<typename> class... Services>
    void Driver<Services...>::start_services() {
        service_stack_.for_each_layer([this](auto&& service) {
            executor_.insert(service);

            service.start();
            while (service.is_service_starting()) {
                step();
            }

            assert(service.is_service_running());
        });

        set_state(State::RUNNING);
    }

    template<template<typename> class... Services>
    void Driver<Services...>::stop_services() {
        service_stack_.for_each_layer([this](auto&& service) {
            service.stop();
            while (service.is_service_stopping()) {
                if (state_ == State::HALTING) {
                    return;
                }

                step();
            }

            // The service should have completed when it stopped.
            assert(service.is_complete());
            assert(service.is_service_stopped());
        });

        set_state(State::STOPPED);
    }

    template<template<typename> class... Services>
    void Driver<Services...>::step() {
        if (executor_.is_runnable()) {
            executor_.run();
        }

        // Poll frequently to drain the completion queue, but don't
        // block if we have more work to do in userspace.
        bool non_blocking = executor_.is_runnable();
        service_stack_.get_bottom_layer().poll(non_blocking);
    }

}
