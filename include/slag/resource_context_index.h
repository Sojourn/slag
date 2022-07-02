#pragma once

#include <vector>
#include <cstddef>
#include "slag/operation_state_machine.h"

namespace slag {

    class ResourceContext;

    template<OperationAction operation_action>
    class ResourceContextIndex {
    public:
        class Cursor {
            Cursor(ResourceContextIndex& index);

        public:
            Cursor();
            Cursor(Cursor&& other) noexcept;
            Cursor(const Cursor& other);
            ~Cursor();

            Cursor& operator=(Cursor&& rhs) noexcept;
            Cursor& operator=(const Cursor& rhs);

            [[noexcept]] ResourceContext* next_resource_context();
            [[noexcept]] Operation* next_operation();

            void reset();

        private:
            ResourceContextIndex* parent_; // TODO: rename?
            size_t                resource_context_index_; // TODO: rename?
            size_t                operation_index_;
        };

        [[noexcept]] Cursor select();
        void insert(ResourceContext& resource_context);
        void vacuum();
        void truncate();

    private:
        std::vector<ResourceContext*> resource_contexts_;
        size_t                        cursor_count_;
    };

}
