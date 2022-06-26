#include "slag/event_loop.h"

static thread_local slag::EventLoop* local_instance_ = nullptr;

slag::EventLoop& slag::EventLoop::local_instance() {
    assert(local_instance_);
    return *local_instance_;
}

slag::EventLoop::EventLoop() {
    local_instance_ = this;
}

slag::EventLoop::~EventLoop() {
    local_instance_ = nullptr;
}
