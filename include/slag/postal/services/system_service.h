#pragma once

#include "slag/layer.h"
#include "slag/core/executor.h"
#include "slag/postal/service.h"
#include "slag/system/reactor.h"

namespace slag {

    template<typename Stack>
    class SystemService
        : public Service
        , public Layer<SystemService, Stack>
    {
    public:
        using Base = Layer<SystemService, Stack>;

        using Base::above;
        using Base::below;

    public:
        SystemService();

        void start();
        void stop();

        void poll(bool non_blocking);

    public:
        Event& runnable_event() override final;

        void run() override final;

    private:
        Executor executor_;
        Reactor  reactor_;
        Event    activate_event_;
    };

}

#include "system_service.hpp"
