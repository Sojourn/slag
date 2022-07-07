#include "slag/io_uring_reactor.h"
#include "slag/resource.h"
#include <liburing/io_uring.h>
#include <stdexcept>

slag::IOURingReactor::IOURingReactor()
    : resource_context_allocator_{1024} // TODO: make configurable
{
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
    Reactor::startup();
}

void slag::IOURingReactor::step() {
    process_submissions();
    process_completions();

    Reactor::step();
}

void slag::IOURingReactor::shutdown() {
    Reactor::shutdown();
}

slag::ResourceContext& slag::IOURingReactor::allocate_resource_context(Resource& resource) {
    return resource_context_allocator_.allocate(resource);
}

void slag::IOURingReactor::cleanup_resource_context(ResourceContext& resource_context) {
    Reactor::cleanup_resource_context(resource_context);
}

void slag::IOURingReactor::deallocate_resource_context(ResourceContext& resource_context) {
    resource_context_allocator_.deallocate(
        static_cast<IOURingResourceContext&>(resource_context)
    );
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

                handle_operation_event(*operation, OperationEvent::SUBMISSION);
                ++submission_count;
            }
        }
    }

    if (submission_count > 0) {
        io_uring_submit(&ring_);
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

template<>
void slag::IOURingReactor::submit_operation<slag::OperationType::ASSIGN>(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<OperationType::ASSIGN>& operation_parameters) {
    (void)sqe;
    (void)operation;
    (void)operation_parameters;
}

template<>
void slag::IOURingReactor::submit_operation<slag::OperationType::CLOSE>(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<OperationType::CLOSE>& operation_parameters) {
    (void)sqe;
    (void)operation;
    (void)operation_parameters;
}

template<>
void slag::IOURingReactor::submit_operation<slag::OperationType::CONNECT>(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<OperationType::CONNECT>& operation_parameters) {
    (void)sqe;
    (void)operation;
    (void)operation_parameters;
}

template<>
void slag::IOURingReactor::submit_operation<slag::OperationType::ACCEPT>(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<OperationType::ACCEPT>& operation_parameters) {
    (void)sqe;
    (void)operation;
    (void)operation_parameters;
}

template<>
void slag::IOURingReactor::submit_operation<slag::OperationType::SEND>(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<OperationType::SEND>& operation_parameters) {
    (void)sqe;
    (void)operation;
    (void)operation_parameters;
}

template<>
void slag::IOURingReactor::submit_operation<slag::OperationType::RECEIVE>(struct io_uring_sqe& sqe, Operation& operation, OperationParameters<OperationType::RECEIVE>& operation_parameters) {
    (void)sqe;
    (void)operation;
    (void)operation_parameters;
}

void slag::IOURingReactor::submit_cancel(struct io_uring_sqe& sqe, Operation& operation) {
    io_uring_prep_cancel(&sqe, &operation, 0);
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
            Operation* operation = reinterpret_cast<Operation*>(cqe->user_data);
            operation->visit_parameters([&]<OperationType operation_type>(OperationParameters<operation_type>& operation_parameters) {
                process_operation_completion(*operation, operation_parameters, cqe->res);
            });
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
void slag::IOURingReactor::process_operation_completion(Operation& operation, OperationParameters<operation_type>& operation_parameters, int64_t result) {
    (void)operation_parameters;

    complete_operation(operation, result);
}

template<>
void slag::IOURingReactor::process_operation_completion<slag::OperationType::ACCEPT>(Operation& operation, OperationParameters<OperationType::ACCEPT>& operation_parameters, int64_t result) {
    if (result >= 0) {
        operation_parameters.file_descriptor = FileDescriptor{static_cast<int>(result)};
        result = 0;
    }

    complete_operation(operation, result);
}
