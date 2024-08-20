#include "event_loop.h"
#include "slag/memory.h"
#include "slag/system.h"
#include <stdexcept>

namespace slag {

    EventLoop::EventLoop()
        : reactor_(*this)
        , looping_(false)
        , current_priority_(TaskPriority::IDLE)
    {
    }

    EventLoop::~EventLoop() {
        // TODO: poll the reactor until quiescent.
    }

    void EventLoop::loop() {
        if (looping_) {
            throw std::runtime_error("Already looping");
        }

        looping_ = true;
        while (looping_) {
            bool non_blocking = false;
            non_blocking |= high_priority_executor_.is_runnable();
            non_blocking |= idle_priority_executor_.is_runnable();
            reactor_.poll(non_blocking);

            if (high_priority_executor_.is_runnable()) {
                current_priority_ = TaskPriority::HIGH;
                high_priority_executor_.run();
            }
            else if (idle_priority_executor_.is_runnable()) {
                current_priority_ = TaskPriority::IDLE;
                idle_priority_executor_.run();
            }
        }
    }

    void EventLoop::stop() {
        looping_ = false;
    }

    void EventLoop::schedule(Task& task) {
        TaskPriority priority = task.priority();
        if (priority == TaskPriority::SAME) {
            priority = current_priority_;
        }

        if (priority == TaskPriority::HIGH) {
            high_priority_executor_.schedule(task);
        }
        else {
            idle_priority_executor_.schedule(task);
        }
    }

    void EventLoop::finalize(Buffer& buffer) {
        // TODO: Manage buffer pools.
        delete &buffer;
    }

    void EventLoop::finalize(FileDescriptor& file_descriptor) {
        if (file_descriptor) {
            start_operation<CloseOperation>(file_descriptor.release())->daemonize();
        }

        delete &file_descriptor;
    }

    void EventLoop::finalize(Operation& operation) {
        operation.abandon();

        if (operation.is_daemonized()) {
            // This is a cleanup operation or something that we don't want to cancel.
        }
        else if (operation.is_quiescent()) {
            reactor_.destroy_operation(operation);
        }
        else {
            operation.cancel();
        }
    }

    void EventLoop::handle_interrupt(Interrupt interrupt) {
        (void)interrupt;
    }

}
