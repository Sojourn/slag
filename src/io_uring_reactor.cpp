#include "slag/io_uring_reactor.h"

slag::IOURingReactor::IOURingReactor() {
}

slag::IOURingReactor::~IOURingReactor() {
}

void slag::IOURingReactor::startup() {
}

void slag::IOURingReactor::step() {
#if 0
    size_t submission_count = 0;
    {
        auto cursor = deferred_submit_operation_actions();
        while (ResourceContext* resource_context = cursor.next()) {
        }

        deferred_submit_operation_actions().vacuum();
    }
    if (submission_count > 0) {
    }

    {
        auto cursor = deferred_notify_operation_actions();
        while (ResourceContext* resource_context = cursor.next()) {
            size_t operation_count = resource_context->operations().size();
            for (size_t operation_index = 0; operation_index < operation_count; ++operation_index) {
                Operation* operation = resource_context->operations()[operation_index];
                if (operation->action() != OperationAction::NOTIFY) {
                    continue;
                }

                handle_operation_event(*operation, OperationEvent::NOTIFY);
                if (resource_context->has_resource()) {
                    resource_context->resource().handle_operation_copmlete(*operation);
                }
            }
        }

        deferred_notify_operation_actions().vacuum();
    }

#endif
}

void slag::IOURingReactor::shutdown() {
}
