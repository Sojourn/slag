#include <iostream>
#include "slag/stack.h"
#include "slag/postal/task.h"
#include "slag/postal/proto_task.h"
#include "slag/postal/reactor.h"
#include "slag/postal/pollable.h"
#include "slag/postal/executor.h"
#include "slag/postal/domain.h"

using namespace slag;
using namespace slag::postal;

#define SLAG_SERVICE_TYPES(X) \
    X(SYSTEM)                 \
    X(LOGGER)                 \
    X(CHRONO)                 \
    X(POSTAL)                 \
    X(WORKER)                 \

enum class ServiceType {
#define X(SLAG_SERVICE_TYPE) SLAG_SERVICE_TYPE,
    SLAG_SERVICE_TYPES(X)
#undef X
};

constexpr size_t SERVICE_TYPE_COUNT = 0
#define X(SLAG_SERVICE_TYPE) + 1
    SLAG_SERVICE_TYPES(X)
#undef X
;

enum class ServiceState {
    INITIAL,
    STARTING,
    RUNNING,
    STOPPING,
    STOPPED,
};

class Service : public Task {
public:
    explicit Service(ServiceType type)
        : type_{type}
        , state_{ServiceState::INITIAL}
    {
    }

    ServiceType service_type() const {
        return type_;
    }

    ServiceState service_state() const {
        return state_;
    }

    bool is_service_starting() const {
        return state_ == ServiceState::STARTING;
    }

    bool is_service_running() const {
        return state_ == ServiceState::RUNNING;
    }

    bool is_service_stopping() const {
        return state_ == ServiceState::STOPPING;
    }

    bool is_service_stopped() const {
        return state_ == ServiceState::STOPPED;
    }

protected:
    void set_state(ServiceState state) {
        assert(static_cast<size_t>(state_) + 1 == static_cast<size_t>(state));

        state_ = state;
    }

private:
    ServiceType  type_;
    ServiceState state_;
};

template<typename Stack>
class SystemService
    : public Layer<SystemService, Stack>
    , public Service
{
public:
    using Base = Layer<SystemService, Stack>;

    using Base::above;
    using Base::below;

public:
    SystemService()
        : Service{ServiceType::SYSTEM}
        , reactor_{executor_}
    {
    }

    void start() {
        info("[SystemService] starting");

        set_state(ServiceState::STARTING);
    }

    void stop() {
        info("[SystemService] stopping");

        set_state(ServiceState::STOPPING);
    }

    void poll(bool non_blocking) {
        reactor_.poll(non_blocking);
    }

public:
    Event& runnable_event() override final{
        return executor_.runnable_event();
    }

    void run() override final {
        if (is_service_starting()) {
            set_state(ServiceState::RUNNING);
            return;
        }
        if (is_service_stopping() && reactor_.is_quiescent()) {
            set_state(ServiceState::STOPPED);
            return;
        }

        while (executor_.is_runnable()) {
            executor_.run();
        }
    }

private:
    Executor executor_;
    Reactor  reactor_;
};

// Runs and handles lifetime management of a stack of services.
// Services will be started in forward order, and stopped in reverse order.
template<template<typename> class... Services>
class Driver {
public:
    Driver()
        : state_{State::INITIAL}
    {
    }

    void run() {
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

    // Gracefully shutdown. This can be called once 
    void stop() {
        set_state(State::STOPPING);
    }

    // Shutdown immediately without stopping services. This can be called while
    // already stopping in case that is taking too long.
    void halt() {
        set_state(State::HALTING);
    }

private:
    enum class State {
        INITIAL,
        RUNNING,
        STOPPING,
        HALTING,
        STOPPED,
    };

    constexpr static bool is_valid_transition(State old_state, State new_state) {
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

    void set_state(State state) {
        if (!is_valid_transition(state_, state)) {
            throw std::runtime_error("Invalid state transition");
        }

        state_ = state;
    }

    void start_services() {
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

    void stop_services() {
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

    void step() {
        if (executor_.is_runnable()) {
            executor_.run();
        }

        // Poll frequently to drain the completion queue, but don't
        // block if we have more work to do in userspace.
        bool non_blocking = executor_.is_runnable();
        service_stack_.get_bottom_layer().poll(non_blocking);
    }

private:
    State              state_;
    Executor           executor_;
    Stack<Services...> service_stack_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Empire::Config empire_config;
    empire_config.index = 0;
    Empire empire_{empire_config};

    Nation::Config nation_config;
    nation_config.index                 = 0;
    nation_config.buffer_count          = 16 * 1024 + 1;
    nation_config.region_count          = 1;
    nation_config.parcel_queue_capacity = 512;
    Nation nation_{nation_config};

    Region::Config region_config;
    region_config.index = 0;
    region_config.buffer_range = std::make_pair(0, nation_config.buffer_count);
    Region regino_{region_config};

    Driver<SystemService> driver;
    driver.run();

    return 0;
}
