#include "slag/event_loop.h"
#include <stdexcept>
#include <cassert>

namespace slag {
    thread_local EventLoop* local_event_loop_ = nullptr;
}

slag::EventLoop::EventLoop(std::unique_ptr<Reactor> reactor)
    : platform_{std::make_unique<Platform>()}
    , reactor_{std::move(reactor)}
    , running_{false}
{
    if (local_event_loop_) {
        throw std::runtime_error("A local event loop already exists");
    }

    local_event_loop_ = this;
    reactor_->startup();
}

slag::EventLoop::~EventLoop() {
    reactor_->shutdown();
    local_event_loop_ = nullptr;
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

slag::Platform& slag::EventLoop::platform() {
    return *platform_;
}

slag::Reactor& slag::EventLoop::reactor() {
    return *reactor_;
}

slag::EventLoop& slag::local_event_loop() {
    assert(local_event_loop_);
    return *local_event_loop_;
}
