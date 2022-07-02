#include "slag/resource_context_index.h"
#include <algorithm>

template<slag::OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor::Cursor(ResourceContextIndex& parent)
    : parent_{&parent}
    , index_{0}
{
}

template<slag::OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor::Cursor()
    : parent_{nullptr}
    , index_{0}
{
}

template<slag::OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor::Cursor(Cursor&& other) noexcept
    : parent_{other.parent_}
    , index_{other.index_}
{
    other.parent_ = nullptr;
    other.index_ = 0;
}

template<slag::OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor::~Cursor() {
    reset();
}

template<slag::OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor& slag::ResourceContextIndex<operation_action>::Cursor::operator=(Cursor&& rhs) noexcept {
    if (this != &rhs) {
        reset();

        parent_ = rhs.parent_;
        index_ = 0;

        rhs.parent_ = nullptr;
        rhs.index_ = 0;
    }

    return *this;
}

template<slag::OperationAction operation_action>
slag::ResourceContextIndex::operator() bool const {
    return static_cast<bool>(parent_);
}

template<slag::OperationAction operation_action>
slag::ResourceContext* slag::ResourceContextIndex<operation_action>::Cursor::next() {
    assert(parent_);

    if (parent_->resource_contexts_.size() <= resource_context_index_) {
        reset(); // finished; detach
        return nullptr;
    }

    return parent_->resource_contexts_[resource_context_index_++];
}

template<slag::OperationAction operation_action>
void slag::ResourceContextIndex<operation_action>::Cursor::reset() {
    if (!parent_) {
        return; // not attached
    }

    --parent_->cursor_count_; // decref

    parent_ = nullptr;
    resource_context_index_ = 0;
    operation_index_ = 0;
}

template<slag::OperationAction operation_action>
slag::ResourceContextIndex<operation_action>::Cursor slag::ResourceContextIndex<operation_action>::select() {
    return Cursor{*this};
}

template<slag::OperationAction operation_action>
void slag::ResourceContextIndex<operation_action>::insert(ResourceContext& resource_context) {
    if (resource_context.has_deferred_action(operation_action)) {
        return; // already in the index
    }

    resource_context.set_deferred_action(operation_action);
    resource_contexts_.push_back(&resource_context);
}

template<slag::OperationAction operation_action>
void slag::ResourceContextIndex<operation_action>::vacuum() {
    if (cursor_count_) {
        return; // cannot vacuum while a select is in progress
    }

    for (ResourceContext*& resource_context: resource_contexts_) {
        if (!resource_context.update_deferred_action(operation_action)) {
            resource_context = nullptr;
        }
    }

    auto beg = resource_contexts_.begin();
    auto end = resource_contexts_.end();
    auto pos = std::remove_if(beg, end, []() {
        return !resource_context;
    });

    resource_contexts_.erase(pos, end);
}

template<slag::OperationAction operation_action>
void slag::ResourceContextIndex<operation_action>::truncate() {
    if (cursor_count_) {
        throw std::runtime_error("ResourceContextIndex cannot be truncated while a select is in progress");
    }

    resource_contexts_.clear();
}
