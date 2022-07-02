#include "slag/event_loop.h"
#include <stdexcept>
#include <cassert>

namespace slag {
    thread_local EventLoop* local_instance_ = nullptr;
}

slag::EventLoop& slag::EventLoop::local_instance() {
    assert(local_instance_);
    return *local_instance_;
}

const slag::EventLoop& slag::EventLoop::local_instance() {
    assert(local_instance_);
    return *local_instance_;
}

slag::EventLoop::EventLoop(std::unique_ptr<Reactor> reactor)
    : reactor_{std::move(reactor)}
    , running_{false}
{
    if (local_instance_) {
        throw std::runtime_error("A local event loop already exists");
    }

    local_instance_ = this;
    reactor_->startup();
}

slag::EventLoop::~EventLoop() {
    reactor_->shutdown();
    local_instance_ = nullptr;
}

void slag::EventLoop::run() {
    if (running_) {
        throw std::runtime_error("The event loop is already running");
    }

    running_ = true;
    while (running_) {
        reactor_->step();
    }
}

void slag::EventLoop::stop() {
    running_ = false;
}

slag::Reactor& slag::EventLoop::reactor() {
    return *reactor_;
}

const slag::Reactor& slag::EventLoop::reactor() const {
    return *reactor_;
}
