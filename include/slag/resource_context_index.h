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
        public:
            Cursor();
            Cursor(ResourceContextIndex& parent);
            Cursor(Cursor&& other) noexcept;
            Cursor(const Cursor&) = delete;
            ~Cursor();

            Cursor& operator=(Cursor&& rhs) noexcept;
            Cursor& operator=(const Cursor&) = delete;

            [[noexcept]] ResourceContext* next();

            void reset();

        private:
            ResourceContextIndex* parent_; // TODO: rename?
            size_t                index_; // TODO: rename?
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
