#include "slag/slag.h"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cassert>

using namespace slag;

event_loop_config::event_loop_config() {
    memset(&params_, 0, sizeof(params_));
    params_.sq_entries = (1 << 16) / sizeof(struct io_uring_sqe);
}

const struct io_uring_params& event_loop_config::params() const {
    return params_;
}

event_loop::event_loop(const event_loop_config& config) {
    struct io_uring_params params = config.params();
    if (int err = io_uring_queue_init_params(params.sq_entries, &ring_, &params)) {
        throw std::runtime_error(strerror(-err));
    }
}

event_loop::~event_loop() {
    io_uring_queue_exit(&ring_);
}

void event_loop::step() {
}

void event_loop::loop() {
    assert(!looping_);
}

void event_loop::stop() {
    looping_ = false;
}
