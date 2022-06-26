#pragma once

#include <cstdint>
#include "slag/opreation_types.h"
#include "slag/operation_parameters.h"
#include "slag/operation_state_machine.h"

namespace slag {

    class ResourceContext;

    class Operation {
    public:
        template<OperationType operation_type>
        Operation(ResourceContext& resource_context, void* user_data, OperationParams<operation_type> operation_params);
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
        [[nodiscard]] OperationParameters& operation_parameters();

        template<OperationType operation_type>
        [[nodiscard]] const OperationParameters& operation_parameters() const;

        template<typename Visitor>
        void visit_operation_parameters(Visitor&& visitor);

        template<typename Visitor>
        void visit_operation_parameters(Visitor&& visitor) const;

    private:
        friend class Driver;

        [[nodiscard]] OperationStateMachine& state_machine();
        [[nodiscard]] const OperationStateMachine& state_machine() const;
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

}
