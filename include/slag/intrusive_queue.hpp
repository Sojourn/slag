#include <cassert>

namespace slag {

    inline IntrusiveQueueNode::IntrusiveQueueNode()
        : queue_{nullptr}
        , sequence_{0}
    {
    }

    inline IntrusiveQueueNode::~IntrusiveQueueNode() {
        unlink();
    }

    inline IntrusiveQueueNode::IntrusiveQueueNode(IntrusiveQueueNode&& other)
        : IntrusiveQueueNode{}
    {
        std::swap(queue_, other.queue_);
        std::swap(sequence_, other.sequence_);

        if (is_linked()) {
            queue_->relocated(*this);
        }
    }

    inline IntrusiveQueueNode& IntrusiveQueueNode::operator=(IntrusiveQueueNode&& that) {
        if (this != &that) {
            unlink();

            std::swap(queue_, that.queue_);
            std::swap(sequence_, that.sequence_);

            if (is_linked()) {
                queue_->relocated(*this);
            }
        }

        return *this;
    }

    inline bool IntrusiveQueueNode::is_linked() const {
        return static_cast<bool>(queue_);
    }

    inline void IntrusiveQueueNode::unlink() {
        if (!is_linked()) {
            return;
        }

        queue_->erase(*this);
    }

    inline auto IntrusiveQueueNode::sequence() const -> std::optional<Sequence> {
        if (!is_linked()) {
            return std::nullopt;
        }

        return sequence_;
    }

    inline void IntrusiveQueueNode::attach(IntrusiveQueueBase& queue, Sequence sequence) {
        assert(!is_linked());

        queue_ = &queue;
        sequence_ = sequence;
    }

    inline void IntrusiveQueueNode::detach(IntrusiveQueueBase& queue) {
        assert(is_linked());
        assert(queue_ == &queue);

        queue_ = nullptr;
        sequence_ = 0;
    }

    inline void IntrusiveQueueNode::set_queue(IntrusiveQueueBase& queue) {
        assert(is_linked());
        queue_ = &queue;
    }

    inline void IntrusiveQueueNode::set_sequence(Sequence sequence) {
        assert(is_linked());
        sequence_ = sequence;
    }

    inline IntrusiveQueueBase::IntrusiveQueueBase()
        : head_{0}
        , tail_{0}
        , mask_{0}
        , capacity_{0}
        , tombstone_count_{0}
    {
    }

    inline IntrusiveQueueBase::~IntrusiveQueueBase() {
        clear();
    }

    inline IntrusiveQueueBase::IntrusiveQueueBase(IntrusiveQueueBase&& other)
        : IntrusiveQueueBase()
    {
        std::swap(slots_,           other.slots_);
        std::swap(head_,            other.head_);
        std::swap(tail_,            other.tail_);
        std::swap(mask_,            other.mask_);
        std::swap(capacity_,        other.capacity_);
        std::swap(tombstone_count_, other.tombstone_count_);

        for (Sequence sequence = head_; sequence != tail_; ++sequence) {
            if (Node* node = *get_slot(sequence)) {
                node->set_queue(*this);
            }
        }
    }

    inline IntrusiveQueueBase& IntrusiveQueueBase::operator=(IntrusiveQueueBase&& that) {
        if (this != &that) {
            clear();

            std::swap(slots_,           that.slots_);
            std::swap(head_,            that.head_);
            std::swap(tail_,            that.tail_);
            std::swap(mask_,            that.mask_);
            std::swap(capacity_,        that.capacity_);
            std::swap(tombstone_count_, that.tombstone_count_);

            for (Sequence sequence = head_; sequence != tail_; ++sequence) {
                if (Node* node = *get_slot(sequence)) {
                    node->set_queue(*this);
                }
            }
        } return *this; }

    inline bool IntrusiveQueueBase::is_empty() const {
        return head_ == tail_;
    }

    inline size_t IntrusiveQueueBase::size() const {
        return tail_ - head_;
    }

    inline size_t IntrusiveQueueBase::capacity() const {
        return capacity_;
    }

    inline size_t IntrusiveQueueBase::tombstone_count() const {
        return tombstone_count_;
    }

    inline auto IntrusiveQueueBase::push_front(IntrusiveQueueNode& node) -> Sequence {
        assert(!node.is_linked());

        if (size() == capacity_) {
            reserve(std::max(size_t{16}, capacity_ * 2)); // grow
        }

        Sequence sequence = --head_;
        *get_slot(sequence) = &node;
        node.attach(*this, sequence);
        return sequence;
    }

    inline auto IntrusiveQueueBase::push_back(IntrusiveQueueNode& node) -> Sequence {
        assert(!node.is_linked());

        if (size() == capacity_) {
            reserve(std::max(size_t{16}, capacity_ * 2)); // grow
        }

        Sequence sequence = tail_++;
        *get_slot(sequence) = &node;
        node.attach(*this, sequence);
        return sequence;
    }

    inline IntrusiveQueueNode* IntrusiveQueueBase::pop_front() {
        if (is_empty()) {
            return nullptr;
        }

        Node** slot = get_slot(head_++);
        Node* node = *slot;
        if (node) {
            node->detach(*this);
            *slot = nullptr;
        }
        else {
            assert(tombstone_count_ > 0);
            tombstone_count_ -= 1; // Popped a tombstone.
        }

        return node;
    }

    inline IntrusiveQueueNode* IntrusiveQueueBase::pop_back() {
        if (is_empty()) {
            return nullptr;
        }

        Node** slot = get_slot(--tail_);
        Node* node = *slot;
        if (node) {
            node->detach(*this);
            *slot = nullptr;
        }
        else {
            assert(tombstone_count_ > 0);
            tombstone_count_ -= 1; // Popped a tombstone.
        }

        return node;
    }

    inline IntrusiveQueueNode* IntrusiveQueueBase::peek_front(size_t relative_offset) {
        if (size() <= relative_offset) {
            return nullptr;
        }

        return *get_slot(head_ + relative_offset);
    }

    inline IntrusiveQueueNode* IntrusiveQueueBase::peek_back(size_t relative_offset) {
        if (size() <= relative_offset) {
            return nullptr;
        }

        return *get_slot(tail_ - relative_offset - 1);
    }

    inline void IntrusiveQueueBase::erase(IntrusiveQueueNode& node) {
        assert(node.is_linked());
        erase(*node.sequence());
    }

    inline void IntrusiveQueueBase::erase(Sequence sequence) {
        Node** slot = get_slot(sequence);
        Node* node = *slot;

        if (node) {
            if (node->sequence() == sequence) {
                node->detach(*this);

                *slot = nullptr;
                tombstone_count_ += 1;
            }
            else {
                assert(false); // Something else lives here now?
            }
        }
        else {
            assert(false); // Double free?
        }

        assert(tombstone_count_ <= size());
    }

    inline void IntrusiveQueueBase::clear() {
        for (Sequence sequence = head_; sequence != tail_; ++sequence) {
            Node** slot = get_slot(sequence);
            if (Node* node = *slot) {
                node->detach(*this);
                *slot = nullptr;
            }
        }

        head_            = 0;
        tail_            = 0;
        tombstone_count_ = 0;
    }

    inline void IntrusiveQueueBase::clear_tombstones() {
        while (tombstone_count_) {
            if (Node* node = pop_front()) {
                push_back(*node);
            }
            else {
                tombstone_count_ -= 1;
            }
        }
    }

    inline void IntrusiveQueueBase::reserve(size_t minimum_capacity) {
        if (capacity_ >= minimum_capacity) {
            return;
        }

        size_t capacity = make_capacity(minimum_capacity);
        size_t mask     = capacity - 1;

        // Copy everything (including tombstones) into a new, larger array.
        std::unique_ptr<Node*[]> slots{new Node*[capacity]};
        for (Sequence sequence = head_; sequence != tail_; ++sequence) {
            *get_slot(slots.get(), mask, sequence) = *get_slot(sequence);
        }

        slots_    = std::move(slots);
        capacity_ = capacity;
        mask_     = mask;
        // head_ and tail_ are preserved to avoid invalidating sequences
    }

    inline void IntrusiveQueueBase::relocated(IntrusiveQueueNode& node) {
        assert(node.is_linked());
        *get_slot(*node.sequence()) = &node;
    }

    // TODO: extract [find the next power-of-2 that is >= number] into a utility function
    inline size_t IntrusiveQueueBase::make_capacity(size_t minimum_capacity) {
        size_t capacity = 1;
        while (capacity < minimum_capacity) {
            capacity *= 2;
        }

        return capacity;
    }

    inline auto IntrusiveQueueBase::get_slot(Sequence sequence) -> Node** {
        return get_slot(slots_.get(), mask_, sequence);
    }

    inline auto IntrusiveQueueBase::get_slot(Node** nodes, Sequence mask, Sequence sequence) -> Node** {
        return &nodes[mask & sequence];
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline IntrusiveQueue<T, node_>::IntrusiveQueue(size_t minimum_capacity) {
        if (minimum_capacity > 0) {
            base_.reserve(minimum_capacity);
        }
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline bool IntrusiveQueue<T, node_>::is_empty() const {
        return size() == 0;
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline size_t IntrusiveQueue<T, node_>::size() const {
        return base_.size() - base_.tombstone_count();
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline size_t IntrusiveQueue<T, node_>::capacity() const {
        return base_.capacity();
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline auto IntrusiveQueue<T, node_>::push_front(T& element) -> Sequence {
        return base_.push_front(to_node(element));
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline auto IntrusiveQueue<T, node_>::push_back(T& element) -> Sequence {
        return base_.push_back(to_node(element));
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline T* IntrusiveQueue<T, node_>::pop_front() {
        while (!is_empty()) {
            if (IntrusiveQueueNode* node = base_.pop_front()) {
                if (IntrusiveQueueNode* next_node = base_.peek_front()) {
                    __builtin_prefetch(next_node);
                }

                return from_node(node);
            }
        }

        return nullptr;
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline T* IntrusiveQueue<T, node_>::pop_back() {
        while (!is_empty()) {
            if (IntrusiveQueueNode* node = base_.pop_back()) {
                if (IntrusiveQueueNode* prev_node = base_.peek_back()) {
                    __builtin_prefetch(prev_node);
                }

                return from_node(node);
            }
        }

        return nullptr;
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline void IntrusiveQueue<T, node_>::erase(T& element) {
        base_.erase(to_node(element));

        // The erase inserted a tombstone; check if we should do something about that.
        if (too_many_tombstones()) {
            base_.clear_tombstones();
        }
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline void IntrusiveQueue<T, node_>::erase(Sequence sequence) {
        base_.erase(sequence);

        // The erase inserted a tombstone; check if we should do something about that.
        if (too_many_tombstones()) {
            base_.clear_tombstones();
        }
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline void IntrusiveQueue<T, node_>::clear() {
        base_.clear();
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline void IntrusiveQueue<T, node_>::reserve(size_t minimum_capacity) {
        base_.reserve(minimum_capacity);
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline T& IntrusiveQueue<T, node_>::from_node(IntrusiveQueueNode& node) {
        static const ptrdiff_t node_offset = reinterpret_cast<ptrdiff_t>(
            &to_node(*reinterpret_cast<T*>(NULL))
        );

        return *reinterpret_cast<T*>(reinterpret_cast<std::byte*>(&node) - node_offset);
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline T* IntrusiveQueue<T, node_>::from_node(IntrusiveQueueNode* node) {
        if (!node) {
            return nullptr;
        }

        return &from_node(*node);
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline IntrusiveQueueNode& IntrusiveQueue<T, node_>::to_node(T& element) {
        return element.*node_;
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline IntrusiveQueueNode* IntrusiveQueue<T, node_>::to_node(T* element) {
        if (!element) {
            return nullptr;
        }

        return &to_node(*element);
    }

    template<typename T, IntrusiveQueueNode T::*node_>
    inline bool IntrusiveQueue<T, node_>::too_many_tombstones() const {
        if (base_.tombstone_count() < 32) {
            return false; // A small number of tombstones can be ignored.
        }
        if (base_.tombstone_count() < (size() / 4)) {
            return false; // A small ratio of tombstones can be ignored.
        }

        return true;
    }

}
