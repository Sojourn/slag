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

        if (operation.is_daemonized()) {
            // This is a cleanup operation or something that we don't want to cancel.
        }
        else if (operation.is_quiescent()) {
            delete &operation;
        }
        else {
            operation.cancel();
        }
    }

    bool Reactor::poll(const bool non_blocking) {
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
            if (struct io_uring_sqe* io_sqe = io_uring_get_sqe(&ring_)) {
                prepare_submission(*io_sqe);
            }
            else {
                return count; // Submission queue is full.
            }
        }

        return ready_count;
    }

    void Reactor::prepare_submission(struct io_uring_sqe& io_sqe) {
        Event& event = *pending_submissions_.select();
        Operation& operation = event.cast_user_data<Operation>();

        // Prepare the submission queue entry.
        const OperationKey op_key = submitted_operation_table_.insert(operation);
        operation.prepare(op_key, io_sqe);
        io_uring_sqe_set_data64(&io_sqe, encode_operation_key(op_key));

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

    void Reactor::process_completion(struct io_uring_cqe& io_cqe) {
        const OperationKey op_key = decode_operation_key(io_cqe.user_data);

        // An invalid key is used to distinguish interrupts from operations we have submitted.
        const bool is_interrupt = op_key == OperationKey{};

        if (is_interrupt) {
            process_interrupt_completion(io_cqe, op_key);
        }
        else {
            process_operation_completion(io_cqe, op_key);
        }
    }

    void Reactor::process_operation_completion(struct io_uring_cqe& io_cqe, const OperationKey op_key) {
        const bool more = io_cqe.flags & IORING_CQE_F_MORE;

        Operation& operation = submitted_operation_table_.select(op_key);
        operation.handle_result(op_key, io_cqe.res, more);

        if (!more) {
            submitted_operation_table_.remove(op_key);

            if (operation.is_abandoned() && operation.is_quiescent()) {
                delete &operation;
            }
        }
    }

    void Reactor::process_interrupt_completion(struct io_uring_cqe& io_cqe, const OperationKey) {
        Interrupt interrupt;
        memcpy(&interrupt, &io_cqe.res, sizeof(interrupt));
        interrupt_handler_.handle_interrupt(interrupt);
    }

}
