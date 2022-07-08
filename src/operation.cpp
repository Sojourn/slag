#include "slag/operation.h"
#include <cerrno>
#include <cassert>

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

bool slag::Operation::test_flag(OperationFlag flag) const {
    return flags_.test(to_index(flag));
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

void slag::Operation::set_flag(OperationFlag flag) {
    flags_.set(to_index(flag));
}

void slag::Operation::set_flags(std::initializer_list<OperationFlag> flags) {
    for (OperationFlag flag: flags) {
        set_flag(flag);
    }
}

void slag::Operation::set_result(int64_t result) {
    result_ = result;
}
