#include "slag/operation.h"
#include <cerrno>
#include <cassert>

template<slag::OperationType operation_type>
slag::Operation::Operation(ResourceContext& resource_context, void* user_data, OperationParameters<operation_type> parameters)
    : resource_context_{resource_context}
    , user_data_{user_data}
    , type_{operation_type}
    , result_{0}
{
    new(&parameters_) OperationParameters<operation_type>{std::move(parameters)};
}

slag::Operation::~Operation() {
    assert(state() == OperationState::TERMINAL);

    visit_parameters([]<OperationType type>(OperationParameters<type>& parameters) {
        parameters.~OperationParameters<type>();
    });
}

slag::ResourceContext& slag::Operation::resource_context() {
    return resource_context_;
}

const slag::ResourceContext& slag::Operation::resource_context() const {
    return resource_context_;
}

void* slag::Operation::user_data() {
    return user_data_;
}

const void* slag::Operation::user_data() const {
    return user_data_;
}

slag::OperationType slag::Operation::type() const {
    return type_;
}

slag::OperationState slag::Operation::state() const {
    return state_machine_.state();
}

slag::OperationAction slag::Operation::action() const {
    return state_machine_.action();
}

int64_t slag::Operation::result() const {
    return result_;
}

bool slag::Operation::success() const {
    return result_ >= 0;
}

bool slag::Operation::failure() const {
    return !success() && !failure();
}

bool slag::Operation::canceled() const {
    return result_ == -ECANCELED;
}

slag::OperationStateMachine& slag::Operation::state_machine() {
    return state_machine_;
}

void slag::Operation::set_result(int64_t result) {
    result_ = result;
}
