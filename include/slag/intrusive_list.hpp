#include <cassert>
#include <cstdlib>

inline slag::IntrusiveListNode::IntrusiveListNode()
    : prev_{this}
    , next_{this}
{
}

inline slag::IntrusiveListNode::IntrusiveListNode(IntrusiveListNode&& other) noexcept
    : prev_{this}
    , next_{this}
{
    if (other.is_linked()) {
        prev_ = std::exchange(other.prev_, &other);
        next_ = std::exchange(other.next_, &other);

        prev_->next_ = this;
        next_->prev_ = this;

        assert(is_linked());
    }

    assert(!other.is_linked());
}

inline slag::IntrusiveListNode::~IntrusiveListNode() {
    unlink();
}

inline slag::IntrusiveListNode& slag::IntrusiveListNode::operator=(IntrusiveListNode&& that) noexcept {
    if (this != &that) {
        unlink();

        if (that.is_linked()) {
            prev_ = std::exchange(that.prev_, &that);
            next_ = std::exchange(that.next_, &that);

            prev_->next_ = this;
            next_->prev_ = this;

            assert(is_linked());
        }

        assert(!that.is_linked());
    }

    return *this;
}

inline bool slag::IntrusiveListNode::is_linked() const {
    return prev_ != this;
}

inline void slag::IntrusiveListNode::unlink() {
    if (!is_linked()) {
        return;
    }

    prev_->next_ = next_;
    next_->prev_ = prev_;

    prev_ = this;
    next_ = this;
}

inline void slag::IntrusiveListNode::link_before(IntrusiveListNode& other) {
    if (is_linked()) {
        abort();
    }

    IntrusiveListNode* prev = other.prev_;
    IntrusiveListNode* next = &other;

    prev_ = prev;
    next_ = next;

    prev_->next_ = this;
    next_->prev_ = this;

    assert(prev->is_linked());
    assert(is_linked());
    assert(next->is_linked());
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline slag::IntrusiveList<T, node_>::IntrusiveList(IntrusiveList&& other) noexcept
    : root_{std::move(other.root_)}
{
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline slag::IntrusiveList<T, node_>::~IntrusiveList() {
    while (!is_empty()) {
        erase(front());
    }
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline slag::IntrusiveList<T, node_>& slag::IntrusiveList<T, node_>::operator=(IntrusiveList&& that) noexcept {
    if (this != &that) {
        clear();

        root_ = std::move(that.root_);
    }

    return *this;
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline bool slag::IntrusiveList<T, node_>::is_empty() const {
    return !root_.is_linked();
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline T& slag::IntrusiveList<T, node_>::front() {
    if (is_empty()) {
        abort();
    }

    return from_node(*root_.next_);
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline T& slag::IntrusiveList<T, node_>::back() {
    if (is_empty()) {
        abort();
    }

    return from_node(*root_.prev_);
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline void slag::IntrusiveList<T, node_>::push_front(T& element) {
    IntrusiveListNode& node = to_node(element);
    assert(!node.is_linked());
    node.link_before(*root_.next_);
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline void slag::IntrusiveList<T, node_>::push_back(T& element) {
    IntrusiveListNode& node = to_node(element);
    assert(!node.is_linked());
    node.link_before(root_);
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline T& slag::IntrusiveList<T, node_>::pop_front() {
    T& element = front();
    erase(element);
    return element;
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline T& slag::IntrusiveList<T, node_>::pop_back() {
    T& element = back();
    erase(element);
    return element;
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline void slag::IntrusiveList<T, node_>::erase(T& element) {
    IntrusiveListNode& node = to_node(element);
    assert(node.is_linked());
    node.unlink();
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline void slag::IntrusiveList<T, node_>::clear() {
    while (!is_empty()) {
        erase(front());
    }
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline T& slag::IntrusiveList<T, node_>::from_node(IntrusiveListNode& node) noexcept {
    static const ptrdiff_t node_offset = reinterpret_cast<ptrdiff_t>(
        &to_node(*reinterpret_cast<T*>(NULL))
    );

    return *reinterpret_cast<T*>(reinterpret_cast<std::byte*>(&node) - node_offset);
}

template<typename T, slag::IntrusiveListNode T::*node_>
inline slag::IntrusiveListNode& slag::IntrusiveList<T, node_>::to_node(T& element) noexcept {
    return element.*node_;
}
