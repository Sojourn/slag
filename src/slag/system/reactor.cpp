#include "reactor.h"
#include "slag/thread_context.h"
#include <stdexcept>
#include <numeric>
#include <cstring>
#include <cassert>

namespace slag {

    Reactor::Reactor(InterruptHandler& interrupt_handler)
        : interrupt_handler_(interrupt_handler)
    {
        memset(&ring_, 0, sizeof(ring_));

        int result;
        do {
            result = io_uring_queue_init(4096, &ring_, 0);
        } while ((result < 0) && (errno == EINTR));

        if (result < 0) {
            throw std::runtime_error("Failed to initialize io_uring");
        }
    }

    Reactor::~Reactor() {
        io_uring_queue_exit(&ring_);
    }

    int Reactor::borrow_file_descriptor() {
        return ring_.ring_fd;
    }

    void Reactor::schedule(Operation& operation) {
        pending_submissions_.insert<PollableType::WRITABLE>(operation);
    }

    void Reactor::finalize(Operation& operation) {
        operation.abandon();

        if (!operation.is_daemonized()) {
            if (operation.is_quiescent()) {
                delete &operation;
            }
            else {
                operation.cancel();
            }
        }
    }

    bool Reactor::poll(bool non_blocking) {
        const size_t submission_count = prepare_submissions();

        if (non_blocking) {
            if (submission_count > 0) {
                io_uring_submit(&ring_);
            }
        }
        else {
            io_uring_submit_and_wait(&ring_, 1);
        }

        const size_t completion_count = process_completions();
        return completion_count > 0;
    }

    size_t Reactor::prepare_submissions() {
        const size_t ready_count = pending_submissions_.ready_count();

        for (size_t count = 0; count < ready_count; ++count) {
            if (struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_)) {
                prepare_submission(*sqe);
            }
            else {
                return count; // Submission queue is full.
            }
        }

        return ready_count;
    }

    void Reactor::prepare_submission(struct io_uring_sqe& sqe) {
        Event& event = *pending_submissions_.select();
        Operation& operation = event.cast_user_data<Operation>();

        OperationUserData user_data = {
            .index = submitted_operation_table_.insert(operation),
            .type  = operation.operation_type(),
            .slot  = operation.allocate_slot(),
            .flags = {
                .multishot = false,
                .interrupt = false,
            },
        };

        // Prepare the submission queue entry.
        operation.prepare(sqe, user_data);
        io_uring_sqe_set_data64(&sqe, encode(user_data));

        // Reinsert the operation in case it needs to submit again.
        pending_submissions_.insert<PollableType::WRITABLE>(operation);
    }

    size_t Reactor::process_completions() {
        size_t completion_count = 0;

        std::array<struct io_uring_cqe*, 8> cqe_array;
        while (int count = io_uring_peek_batch_cqe(&ring_, cqe_array.data(), cqe_array.size())) {
            if (count > 0) {
                for (int i = 0; i < count; ++i) {
                    process_completion(*cqe_array[i]);
                }

                io_uring_cq_advance(&ring_, static_cast<unsigned>(count));
                completion_count += static_cast<size_t>(count);
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

        return completion_count;
    }

    void Reactor::process_completion(struct io_uring_cqe& cqe) {
        const OperationUserData user_data = decode(cqe.user_data);

        if (user_data.flags.interrupt) {
            process_interrupt_completion(cqe, user_data);
        }
        else {
            process_operation_completion(cqe, user_data);
        }
    }

    void Reactor::process_operation_completion(struct io_uring_cqe& cqe, const OperationUserData& user_data) {
        const bool more = cqe.flags & IORING_CQE_F_MORE;

        Operation& operation = submitted_operation_table_.select(user_data.index);
        operation.handle_result(cqe.res, more, user_data);

        if (!more) {
            submitted_operation_table_.remove(user_data.index);

            operation.deallocate_slot(user_data.slot);
            if (operation.is_abandoned() && operation.is_quiescent()) {
                delete &operation;
            }
        }
    }

    void Reactor::process_interrupt_completion(struct io_uring_cqe& cqe, const OperationUserData& user_data) {
        (void)user_data;

        Interrupt interrupt;
        memcpy(&interrupt, &cqe.res, sizeof(interrupt));
        interrupt_handler_.handle_interrupt(interrupt);
    }

}
