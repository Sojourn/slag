#include "slag/postal/reactor.h"
#include "slag/core/domain.h"
#include <stdexcept>
#include <cassert>

namespace slag {

    Reactor::Reactor(Executor& executor)
        : executor_{executor}
        , interrupt_handler_{nullptr}
    {
        int result;
        do {
            result = io_uring_queue_init(4096, &ring_, 0);
        } while ((result < 0) && (errno == EINTR));

        if (result < 0) {
            throw std::runtime_error("Failed to initialize io_uring");
        }

        region().attach_reactor(*this);
    }

    Reactor::~Reactor() {
        io_uring_queue_exit(&ring_);

        region().detach_reactor(*this);
    }

    void Reactor::poll(bool non_blocking) {
        size_t submission_count = prepare_pending_operations();

        if (non_blocking) {
            if (submission_count > 0) {
                io_uring_submit(&ring_);
            }
            else {
                // Proceed to checking for completions.
            }
        }
        else {
            if (submission_count > 0) {
                io_uring_submit_and_wait(&ring_, 1);
            }
            else {
                // TODO: check if there is version of this that doesn't submit.
                io_uring_submit_and_wait(&ring_, 1);
            }
        }

        process_completions();
        collect_garbage();
    }

    bool Reactor::is_quiescent() const {
        return metrics_.active_operation_count == 0;
    }

    void Reactor::set_interrupt_handler(InterruptHandler& interrupt_handler) {
        interrupt_handler_ = &interrupt_handler;
    }

    int Reactor::file_descriptor() {
        return ring_.ring_fd;
    }

    size_t Reactor::prepare_pending_operations() {
        size_t count = 0;
        bool done = false;

        do {
            if (Event* event = pending_submissions_.select()) {
                OperationBase* operation_base = reinterpret_cast<OperationBase*>(
                    event->user_data()
                );

                if (auto&& [user_data, slot] = operation_base->produce_slot(); user_data) {
                    // Attempt to prepare a new submission queue entry.
                    if (struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_)) {
                        operation_base->prepare(slot, *sqe);
                        io_uring_sqe_set_data(sqe, user_data);

                        count += 1;
                    }
                    else {
                        // The submisssion queue is full.
                        done = true;
                    }
                }
                else {
                    // This operation has the maximum number of in-flight requests.
                    assert(false);
                    done = true;
                }

                pending_submissions_.insert<PollableType::WRITABLE>(*operation_base);
            }
            else {
                done = true;
            }
        } while (!done);

        return count;
    }

    void Reactor::process_completions() {
        std::array<struct io_uring_cqe*, 8> cqe_array;

        while (int count = io_uring_peek_batch_cqe(&ring_, cqe_array.data(), cqe_array.size())) {
            if (count > 0) {
                for (int i = 0; i < count; ++i) {
                    process_completion(*cqe_array[i]);
                }

                io_uring_cq_advance(&ring_, static_cast<unsigned>(count));
            }
            else {
                if (errno == EINTR) {
                    continue; // Interrupted. Try again.
                }

                // Look at the error and add handling if it is recoverable.
                // I think this is just reading the published sequence number
                // and some memory fencing--shouldn't have many failure modes...
                abort();
            }
        }
    }

    void Reactor::process_completion(struct io_uring_cqe& cqe) {
        if (cqe.user_data) {
            process_operation_completion(cqe);
        }
        else {
            process_interrupt_completion(cqe);
        }
    }

    void Reactor::process_operation_completion(struct io_uring_cqe& cqe) {
        bool more = cqe.flags & IORING_CQE_F_MORE;
        void* user_data = reinterpret_cast<void*>(cqe.user_data);
        auto&& [operation_base, slot] = (more ? OperationBase::peek_slot(user_data) : OperationBase::consume_slot(user_data));

        operation_base->handle_result(slot, cqe.res, more);
    }

    void Reactor::process_interrupt_completion(struct io_uring_cqe& cqe) {
        if (!interrupt_handler_) {
            return;
        }

        // Extract the payload from the completion entry.
        InterruptOperationPayload payload;
        memcpy(&payload, &cqe.res, sizeof(payload));

        interrupt_handler_->handle_interrupt(payload.source, payload.reason);
    }

    void Reactor::collect_garbage() {
        while (Event* event = garbage_.select()) {
            void* user_data = event->user_data();

            visit([&](auto&& operation) {
                operation_allocator_.deallocate(operation);
                metrics_.active_operation_count -= 1;
            }, *reinterpret_cast<OperationBase*>(user_data));
        }
    }

    void Reactor::handle_abandoned(OperationBase& operation_base) {
        visit([&](auto&& operation) {
            Event& event = operation.complete_event();

            // Adopt the operation so that we can reap it after it has completed.
            event.set_user_data(&operation_base);
            if (event.is_linked()) {
                event.unlink();
            }

            garbage_.insert(event);
        }, operation_base);
    }

}
