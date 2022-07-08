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
    size_t prepared_submission_count = 0;
    bool ok = true;

    auto cursor = deferred_submit_operation_actions();
    while (ResourceContext* resource_context = cursor.next()) {
        auto&& operations = resource_context->operations();
        size_t operation_count = operations.size();
        for (size_t operation_index = 0; operation_index < operation_count; ++operation_index) {
            Operation& operation = *operations[operation_index];
            if (operation.action() != OperationAction::SUBMIT) {
                continue;
            }

            operation.visit_parameters([&]<OperationType operation_type>(OperationParameters<operation_type>& operation_parameters) {
                Subject<operation_type> subject = {
                    .resource_context     = static_cast<IOURingResourceContext&>(*resource_context),
                    .operation            = operation,
                    .operation_parameters = operation_parameters,
                };

                switch (operation.state()) {
                    case OperationState::SPOOLED: {
                        ok = prepare_submission(subject);
                        break;
                    }
                    case OperationState::CANCEL_SPOOLED: {
                        ok = prepare_cancel_submission(subject);
                        break;
                    }
                    default: {
                        abort();
                    }
                }
            });
            if (!ok) {
                break;
            }

            ++prepared_submission_count;
        }
        if (!ok) {
            break;
        }
    }

    if (prepared_submission_count > 0) {
        io_uring_submit(&ring_);
    }
}

template<slag::OperationType operation_type>
bool slag::IOURingReactor::prepare_submission(Subject<operation_type>& subject) {
    (void)subject;

    abort();
}

template<>
bool slag::IOURingReactor::prepare_submission<slag::OperationType::NOP>(Subject<OperationType::NOP>& subject) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    if (!sqe) {
        return false;
    }

    io_uring_prep_nop(sqe);
    io_uring_sqe_set_data(sqe, &subject.operation);

    handle_operation_event(subject.operation, OperationEvent::SUBMISSION);
    return true;
}

template<>
bool slag::IOURingReactor::prepare_submission<slag::OperationType::ASSIGN>(Subject<OperationType::ASSIGN>& subject) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    if (!sqe) {
        return false;
    }

    FileDescriptor& source_file_descriptor = subject.operation_parameters.file_descriptor;
    FileDescriptor& target_file_descriptor = subject.resource_context.file_descriptor();

    int64_t result = 0;
    if (!source_file_descriptor.is_open()) {
        result = -EINVAL;
    }
    else if (target_file_descriptor.is_open()) {
        result = -EINVAL;
    }
    else {
        target_file_descriptor = std::move(source_file_descriptor);
    }

    handle_operation_event(subject.operation, OperationEvent::SUBMISSION);
    process_completion(subject, result);
    return true;
}

template<slag::OperationType operation_type>
bool slag::IOURingReactor::prepare_cancel_submission(Subject<operation_type>& subject) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    if (!sqe) {
        return false;
    }

    io_uring_prep_cancel(sqe, &subject.operation, 0);

    handle_operation_event(subject.operation, OperationEvent::SUBMISSION);
    return true;
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
                Subject subject = {
                    .resource_context     = static_cast<IOURingResourceContext&>(operation->resource_context()),
                    .operation            = *operation,
                    .operation_parameters = operation_parameters,
                };

                process_completion(subject, cqe->res);
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
void slag::IOURingReactor::process_completion(Subject<operation_type>& subject, int64_t result) {
    complete_operation(subject.operation, result);
}

template<>
void slag::IOURingReactor::process_completion<slag::OperationType::ACCEPT>(Subject<OperationType::ACCEPT>& subject, int64_t result) {
    if (result >= 0) {
        subject.operation_parameters.file_descriptor = FileDescriptor{static_cast<int>(result)};
        result = 0;
    }

    complete_operation(subject.operation, result);
}
