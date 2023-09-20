#include <iostream>
#include "slag/stack.h"
#include "slag/postal/task.h"
#include "slag/postal/proto_task.h"
#include "slag/postal/reactor.h"
#include "slag/postal/pollable.h"
#include "slag/postal/executor.h"
#include "slag/postal/domain.h"
#include "slag/postal/driver.h"
#include "slag/postal/service.h"

using namespace slag;
using namespace slag::postal;

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
