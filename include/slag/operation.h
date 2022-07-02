#pragma once

#include <cstdint>
#include "slag/operation_types.h"
#include "slag/operation_parameters.h"
#include "slag/operation_state_machine.h"

namespace slag {

    class ResourceContext;

    enum class OperationFlags {
        // linked, wait_for_previous, etc.
    };

    class Operation {
    public:
        template<OperationType operation_type>
        Operation(ResourceContext& resource_context, void* user_data, OperationParameters<operation_type> operation_params);
        Operation(Operation&&) noexcept = delete;
        Operation(const Operation&) = delete;
        ~Operation();

        Operation& operator=(Operation&&) noexcept = delete;
        Operation& operator=(const Operation&) = delete;

        [[nodiscard]] ResourceContext& resource_context();
        [[nodiscard]] const ResourceContext& resource_context() const;
        [[nodiscard]] void* user_data();
        [[nodiscard]] const void* user_data() const;
        [[nodiscard]] OperationType type() const;
        [[nodiscard]] OperationState state() const;
        [[nodiscard]] OperationAction action() const;
        [[nodiscard]] int64_t result() const;
        [[nodiscard]] bool success() const;
        [[nodiscard]] bool failure() const;
        [[nodiscard]] bool canceled() const;

        template<OperationType operation_type>
        [[nodiscard]] OperationParameters<operation_type>& parameters();

        template<OperationType operation_type>
        [[nodiscard]] const OperationParameters<operation_type>& parameters() const;

        template<typename Visitor>
        void visit_parameters(Visitor&& visitor);

        template<typename Visitor>
        void visit_parameters(Visitor&& visitor) const;

    private:
        friend class Reactor;

        [[nodiscard]] OperationStateMachine& state_machine();
        void set_result(int64_t result);

    private:
        ResourceContext&      resource_context_;
        void*                 user_data_;
        OperationStateMachine state_machine_;
        OperationType         type_;
        int64_t               result_;

        std::aligned_storage_t<
            max_operation_parameters_size(),
            max_operation_parameters_alignment()
        > parameters_;
    };

    template<OperationType operation_type>
    inline OperationParameters<operation_type>& Operation::parameters() {
        assert(type_ == operation_type);
        return *reinterpret_cast<OperationParameters<operation_type>*>(&parameters_);
    }

    template<OperationType operation_type>
    inline const OperationParameters<operation_type>& Operation::parameters() const {
        assert(type_ == operation_type);
        return *reinterpret_cast<const OperationParameters<operation_type>*>(&parameters_);
    }

    template<typename Visitor>
    inline void Operation::visit_parameters(Visitor&& visitor) {
        switch (type_) {
#define X(SLAG_OPERATION_TYPE)                                             \
            case OperationType::SLAG_OPERATION_TYPE: {                     \
                visitor(parameters<OperationType::SLAG_OPERATION_TYPE>()); \
                break;                                                     \
            }

            SLAG_OPERATION_TYPES(X)
#undef X
        }
    }

    template<typename Visitor>
    inline void Operation::visit_parameters(Visitor&& visitor) const {
        switch (type_) {
#define X(SLAG_OPERATION_TYPE)                                             \
            case OperationType::SLAG_OPERATION_TYPE: {                     \
                visitor(parameters<OperationType::SLAG_OPERATION_TYPE>()); \
                break;                                                     \
            }

            SLAG_OPERATION_TYPES(X)
#undef X
        }
    }
}
