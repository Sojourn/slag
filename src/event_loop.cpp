#include "slag/event_loop.h"
#include "slag/system/system_service_interface.h"
#include "slag/scheduling/scheduler_service_interface.h"
#include <cassert>

namespace slag {

    thread_local EventLoop* event_loop_instance = nullptr;

    class [[nodiscard]] EventLoopActivation {
        EventLoopActivation(EventLoopActivation&&) = delete;
        EventLoopActivation(const EventLoopActivation&) = delete;
        EventLoopActivation& operator=(EventLoopActivation&&) = delete;
        EventLoopActivation& operator=(const EventLoopActivation&) = delete;

    public:
        explicit EventLoopActivation(EventLoop& event_loop)
            : event_loop_{event_loop_instance}
        {
            assert(!event_loop.is_stepping());

            event_loop_instance = &event_loop;
            event_loop_instance->set_stepping();
        }

        ~EventLoopActivation() {
            event_loop_instance->reset_stepping();
            event_loop_instance = event_loop_;
        }

    private:
        EventLoop* event_loop_;
    };

    EventLoop::EventLoop(ServiceRegistry& service_registry)
        : service_registry_{service_registry}
        , spinning_{false}
        , stepping_{false}
        , stopping_{false}
        , idle_streak_{0}
    {
    }

    ServiceRegistry& EventLoop::service_registry() {
        return service_registry_;
    }

    const ServiceRegistry& EventLoop::service_registry() const {
        return service_registry_;
    }

    bool EventLoop::is_spining() const {
        return spinning_;
    }

    void EventLoop::set_spinning() {
        spinning_ = true;
    }

    void EventLoop::reset_spinning() {
        spinning_ = false;
    }

    void EventLoop::loop() {
        assert(!stepping_);
        assert(!stopping_);

        // Guard against reentrancy.
        {
            EventLoopActivation activation{*this};

            while (!stopping_) {
                step_unsafe();
            }
        }
    }

    bool EventLoop::step() {
        assert(!stepping_);
        assert(!stopping_);

        // Guard against reentrancy.
        {
            EventLoopActivation activation{*this};

            step_unsafe();
        }

        bool running = !stopping_;
        return running;
    }

    void EventLoop::step_unsafe() {
        get_scheduler_service(service_registry_).run();

        bool non_blocking = !should_block();
        if (get_system_service(service_registry_).poll(non_blocking)) {
            idle_streak_ = 0;
        }
        else if (non_blocking) {
            idle_streak_ += 1;
        }
        else {
            // We were blocking and got spuriously woken up. Explicitly
            // breaking out this case causes idle_streak_ to be bounded.
        }
    }

    void EventLoop::stop() {
        stopping_ = true;
    }

    bool EventLoop::is_stepping() const {
        return stepping_;
    }

    void EventLoop::set_stepping() {
        stepping_ = true;
    }

    void EventLoop::reset_stepping() {
        stepping_ = false;
    }

    bool EventLoop::can_block() const {
        if (spinning_) {
            // Spinning implies that we won't block.
            return false;
        }

        if (stopping_) {
            // Can't block if we are stopping.
           return false;
        }

        if (!get_scheduler_service(service_registry_).is_quiescent()) {
            // Can't block if the scheduler has work to do in userspace.
            return false;
        }

        // Block by default to be polite.
        return true;
    }

    bool EventLoop::should_block() const {
        if (idle_streak_ < MAX_IDLE_STREAK) {
            // We are either busy or have recently been woken up.
            return false;
        }

        // If we can't block, probably shouldn't block.
        return can_block();
    }

    EventLoop& get_event_loop() {
        assert(event_loop_instance);
        return *event_loop_instance;
    }

}
