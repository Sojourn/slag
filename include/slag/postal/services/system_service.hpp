#include <cassert>

namespace slag {

    template<typename Stack>
    inline SystemService<Stack>::SystemService()
        : Service{ServiceType::SYSTEM}
        , reactor_{executor_}
    {
    }

    template<typename Stack>
    inline void SystemService<Stack>::start() {
        info("[SystemService] starting");

        set_service_state(ServiceState::STARTING);
        activate_event_.set(); // Force ourselves to wake up.
    }

    template<typename Stack>
    inline void SystemService<Stack>::stop() {
        info("[SystemService] stopping");

        set_service_state(ServiceState::STOPPING);
        activate_event_.set(); // Force ourselves to wake up.
    }

    template<typename Stack>
    inline void SystemService<Stack>::poll(bool non_blocking) {
        reactor_.poll(non_blocking);
    }

    template<typename Stack>
    inline Event& SystemService<Stack>::runnable_event() {
        if (activate_event_.is_set()) {
            return activate_event_;
        }

        return executor_.runnable_event();
    }

    template<typename Stack>
    inline void SystemService<Stack>::run() {
        // Consume the event.
        if (activate_event_.is_set()) {
            activate_event_.reset();
        }

        if (is_service_starting()) {
            set_service_state(ServiceState::RUNNING);
            return;
        }
        if (is_service_stopping()) {
            if (reactor_.is_quiescent()) {
                set_success();
                set_service_state(ServiceState::STOPPED);
                return;
            }
            else {
                asm("int $3");
            }
        }

        while (executor_.is_runnable()) {
            executor_.run();
        }
    }

}
