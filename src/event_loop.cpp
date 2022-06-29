#include "slag/event_loop.h"

namespace slag {
    thread_local slag::EventLoop* local_instance_ = nullptr;
}

slag::EventLoop& slag::EventLoop::local_instance() {
    assert(local_instance_);
    return *local_instance_;
}

const slag::EventLoop& slag::EventLoop::local_instance() {
    assert(local_instance_);
    return *local_instance_;
}

slag::EventLoop::EventLoop() {
    if (local_instance_) {
        throw std::runtime_error("A local event loop already exists");
    }

    local_instance_ = this;
}

slag::EventLoop::~EventLoop() {
    local_instance_ = nullptr;
}
