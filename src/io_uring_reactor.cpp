#include "slag/io_uring_reactor.h"
#include "slag/resource.h"
#include <liburing/io_uring.h>
#include <stdexcept>

slag::IOURingReactor::IOURingReactor() {
    int result;
    do {
        result = io_uring_queue_init(4096, &ring_, 0);
    } while ((result < 0) && (errno == EINTR));

    if (result < 0) {
        throw std::runtime_error("Failed to initialize io_uring");
    }
}

slag::IOURingReactor::~IOURingReactor() {
    io_uring_queue_exit(&ring_);
}

void slag::IOURingReactor::startup() {
}

void slag::IOURingReactor::step() {
    process_submissions();
    process_completions();
}

void slag::IOURingReactor::shutdown() {
}

void slag::IOURingReactor::process_submissions() {
    size_t submission_count = 0;
    bool busy = false;
    {
        auto cursor = deferred_submit_operation_actions();
        ResourceContext* resource_context = nullptr;
        while (!busy && (resource_context = cursor.next())) {
            auto&& operations = resource_context->operations();
            size_t operation_count = operations.size();
            for (size_t operation_index = 0; operation_index < operation_count; ++operation_index) {
                Operation* operation = resource_context->operations()[operation_index];
                if (operation->action() != OperationAction::SUBMIT) {
                    continue;
                }

                struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
                if (!sqe) {
                    busy = true;
                    break;
                }

                operation->visit_parameters([&]<OperationType operation_type>(OperationParameters<operation_type>& operation_parameters) {
                    submit_operation(*sqe, *operation, operation_parameters);
                });

                ++submission_count;
            }
        }
    }

    if (submission_count > 0) {
        io_uring_submit(&ring_);
    }
}

void slag::IOURingReactor::process_completions() {
    struct io_uring_cqe* cqes[64];
    int count = 0;
    do {
        count = io_uring_peek_batch_cqe(&ring_, cqes, 64);
    } while (((count < 0) && (errno == EINTR)));

    if (count < 0) {
        throw std::runtime_error("Failed to peek at cqe ring");
    }
    else if (count > 0) {
        for (int i = 0; i < count; ++i) {
            struct io_uring_cqe* cqe = cqes[i];

            (void)cqe; // TODO: handle cqe
        }

        io_uring_cq_advance(&ring_, static_cast<unsigned>(count));
    }

    auto cursor = deferred_notify_operation_actions();
    while (ResourceContext* resource_context = cursor.next()) {
        size_t operation_count = resource_context->operations().size();
        for (size_t operation_index = 0; operation_index < operation_count; ++operation_index) {
            Operation* operation = resource_context->operations()[operation_index];
            if (operation->action() != OperationAction::NOTIFY) {
                continue;
            }

            handle_operation_event(*operation, OperationEvent::NOTIFICATION);
            if (resource_context->has_resource()) {
                resource_context->resource().handle_operation_complete(*operation);
            }
        }
    }
}

template<slag::OperationType operation_type>
void slag::IOURingReactor::submit_operation(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<operation_type>& operation_parameters) {
    (void)sqe;
    (void)operation;
    (void)operation_parameters;

    abort();
}

template<>
void slag::IOURingReactor::submit_operation<slag::OperationType::NOP>(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<OperationType::NOP>& operation_parameters) {
    (void)operation_parameters;

    io_uring_prep_nop(&sqe);
    io_uring_sqe_set_data(&sqe, &operation);
}

void slag::IOURingReactor::submit_cancel(struct io_uring_sqe& sqe, Operation& operation) {
    io_uring_prep_cancel(&sqe, &operation, 0);
}