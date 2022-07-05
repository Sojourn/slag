#include "slag/resource_context_index.h"
#include "slag/resource_context.h"
#include <algorithm>

slag::ResourceContextIndex::Cursor::Cursor(ResourceContextIndex& parent)
    : parent_{&parent}
    , offset_{0}
{
}

slag::ResourceContextIndex::Cursor::Cursor()
    : parent_{nullptr}
    , offset_{0}
{
}

slag::ResourceContextIndex::Cursor::Cursor(Cursor&& other) noexcept
    : parent_{other.parent_}
    , offset_{other.offset_}
{
    other.parent_ = nullptr;
    other.offset_ = 0;
}

slag::ResourceContextIndex::Cursor::~Cursor() {
    reset();
}

slag::ResourceContextIndex::Cursor& slag::ResourceContextIndex::Cursor::operator=(Cursor&& rhs) noexcept {
    if (this != &rhs) {
        reset();

        parent_ = rhs.parent_;
        offset_ = 0;

        rhs.parent_ = nullptr;
        rhs.offset_ = 0;
    }

    return *this;
}

slag::ResourceContext* slag::ResourceContextIndex::Cursor::next() {
    assert(parent_);

    if (parent_->resource_contexts_.size() <= offset_) {
        reset(); // finished; detach
        return nullptr;
    }

    return parent_->resource_contexts_[offset_++];
}

void slag::ResourceContextIndex::Cursor::reset() {
    if (!parent_) {
        return; // not attached
    }

    --parent_->cursor_count_; // decref
    parent_ = nullptr;
    offset_ = 0;
}

slag::ResourceContextIndex::ResourceContextIndex(OperationAction operation_action)
    : operation_action_{operation_action}
    , cursor_count_{0}
{
}

slag::ResourceContextIndex::Cursor slag::ResourceContextIndex::select() {
    return Cursor{*this};
}

void slag::ResourceContextIndex::insert(ResourceContext& resource_context) {
    if (resource_context.has_deferred_action(operation_action_)) {
        return; // already in the index
    }

    resource_context.set_deferred_action(operation_action_);
    resource_contexts_.push_back(&resource_context);
}

void slag::ResourceContextIndex::vacuum() {
    if (cursor_count_) {
        return; // cannot vacuum while a select is in progress
    }

    for (ResourceContext*& resource_context: resource_contexts_) {
        if (!resource_context->update_deferred_action(operation_action_)) {
            resource_context = nullptr;
        }
    }

    auto beg = resource_contexts_.begin();
    auto end = resource_contexts_.end();
    auto pos = std::remove_if(beg, end, [](ResourceContext* resource_context) {
        return !resource_context;
    });

    resource_contexts_.erase(pos, end);
}

void slag::ResourceContextIndex::truncate() {
    if (cursor_count_) {
        throw std::runtime_error("ResourceContextIndex cannot be truncated while a select is in progress");
    }

    resource_contexts_.clear();
}
