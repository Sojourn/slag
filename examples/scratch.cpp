#include <iostream>
#include "slag/stack.h"
#include "slag/postal/task.h"
#include "slag/postal/proto_task.h"
#include "slag/postal/reactor.h"
#include "slag/postal/pollable.h"
#include "slag/postal/executor.h"

using namespace slag;
using namespace slag::postal;

#define SLAG_SERVICE_TYPES(X) \
    X(SYSTEM)                 \
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

class ServiceBase {
public:
    virtual ~ServiceBase() = default;

    ServiceState state() const {
        return state_;
    }

protected:
    void set_state(ServiceState state) {
        assert((static_cast<size_t>(state_) + 1) == static_cast<size_t>(state));
    }

private:
    ServiceState state_;
};

template<typename Impl, ServiceType type>
class Service;

template<typename Impl>
class Service<Impl, ServiceType::SYSTEM> : public ServiceBase {
public:
    constexpr static ServiceType TYPE = ServiceType::SYSTEM;

private:
    Impl& impl() {
        return static_cast<Impl&>(*this);
    }
};

template<typename Stack>
class FooService
    : public Service<FooService<Stack>, ServiceType::SYSTEM>
    , public Layer<FooService, Stack>
    , public ProtoTask
{
public:
    using Base = Layer<FooService, Stack>;

    using Base::above;
    using Base::below;

public:
    FooService() {
    }

    void start() {
    }

    void stop() {
    }

    void wait() {
    }

private:
    Event runnable_event_;
};

template<typename Stack>
class BarService
    : public Service<BarService<Stack>, ServiceType::CHRONO>
    , public Layer<BarService, Stack>
    , public ProtoTask
{
public:
    using Base = Layer<BarService, Stack>;

    using Base::above;
    using Base::below;

public:
    BarService();
};

// Runs a stack of services.
template<template<typename> class... Services>
class Driver {
public:
    Driver()
        : running_{false}
        , stopping_{false}
    {
    }

    void run() {
        if (!running_) {
            throw std::runtime_error("Already running");
        }
        if (stopping_) {
            throw std::runtime_error("Already stopped");
        }

        running_ = true;
        {
            start_services();

            // Run until something requests that we stop.
            while (!stopping_) {
                step();
            }

            stop_services();
        }
        running_ = false;
    }

    void stop() {
        assert(running_);

        stopping_ = true;
    }

private:
    void start_services() {
        service_stack_.for_each_layer([this](auto&& service) {
            executor_.insert(service);

            service.start();
            while (service.state() != ServiceState::RUNNING) {
                step();
            }
        });
    }

    void stop_services() {
        service_stack_.for_each_layer([this](auto&& service) {
            service.stop();
            while (service.state() != ServiceState::RUNNING) {
                step();
            }

            assert(service.is_complete());
        });
    }

    void step() {
        if (executor_.is_runnable()) {
            executor_.run();
        }
        else {
            service_stack_.get_bottom_layer().wait();
        }
    }

private:
    Executor           executor_;
    bool               running_;
    bool               stopping_;
    Stack<Services...> service_stack_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Driver<FooService, BarService> driver;

    return 0;
}
