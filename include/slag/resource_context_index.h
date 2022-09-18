#pragma once

#include <vector>
#include <cstddef>
#include "slag/operation_state_machine.h"

namespace slag {

    class ResourceContext;

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

            [[nodiscard]] ResourceContext* next();

            void reset();

        private:
            ResourceContextIndex* parent_;
            size_t                offset_;
        };

        ResourceContextIndex(OperationAction operation_action);
        ResourceContextIndex(ResourceContextIndex&&) noexcept = delete;
        ResourceContextIndex(const ResourceContextIndex&) = delete;

        ResourceContextIndex& operator=(ResourceContextIndex&&) noexcept = delete;
        ResourceContextIndex& operator=(const ResourceContextIndex&) = delete;

        [[nodiscard]] Cursor select();
        void insert(ResourceContext& resource_context);
        void vacuum();
        void truncate();

    private:
        OperationAction               operation_action_;
        size_t                        cursor_count_;
        std::vector<ResourceContext*> resource_contexts_;
    };

}
