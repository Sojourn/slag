#pragma once

#include <array>
#include <liburing.h>
#include "slag/core/selector.h"
#include "slag/system/operation.h"
#include "slag/system/operation_handle.h"
#include "slag/system/operation_allocator.h"

namespace slag {

    // TODO: move this into a seperate class and make a reason enum.
    class InterruptHandler {
    protected:
        ~InterruptHandler() = default;

    public:
        virtual void handle_interrupt(uint16_t source, uint16_t reason) = 0;
    };

    class Reactor {
        Reactor(Reactor&&) = delete;
        Reactor(const Reactor&) = delete;
        Reactor& operator=(Reactor&&) = delete;
        Reactor& operator=(const Reactor&) = delete;

    public:
        explicit Reactor(Selector& pending_submissions);
        ~Reactor();

        void set_interrupt_handler(InterruptHandler& interrupt_handler);

        int borrow_file_descriptor();

        // Returns true if there were completions.
        bool poll(bool non_blocking);

    private:
        size_t prepare_pending_operations();
        size_t process_completions();
        void process_completion(struct io_uring_cqe& cqe);

        void process_operation_completion(struct io_uring_cqe& cqe);
        void process_interrupt_completion(struct io_uring_cqe& cqe);

    private:
        struct io_uring   ring_;
        Selector&         pending_submissions_;
        InterruptHandler* interrupt_handler_;
    };

}
