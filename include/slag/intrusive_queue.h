#pragma once

#include <optional>
#include <iterator>
#include <array>
#include <memory>
#include <cstdint>
#include <cstddef>

namespace slag {

    // Considerations:
    //   - Iterators?

    class IntrusiveQueueBase;

    using IntrusiveQueueSequence = size_t;

    class IntrusiveQueueNode {
    public:
        using Sequence = IntrusiveQueueSequence;

        IntrusiveQueueNode();
        ~IntrusiveQueueNode();

        IntrusiveQueueNode(IntrusiveQueueNode&& other);
        IntrusiveQueueNode(const IntrusiveQueueNode&) = delete;
        IntrusiveQueueNode& operator=(IntrusiveQueueNode&& that);
        IntrusiveQueueNode& operator=(const IntrusiveQueueNode&) = delete;

        bool is_linked() const;
        void unlink();

    private:
        friend class IntrusiveQueueBase;

        std::optional<Sequence> sequence() const;

        void attach(IntrusiveQueueBase& queue, Sequence sequence);
        void detach(IntrusiveQueueBase& queue);

        void set_queue(IntrusiveQueueBase& queue);
        void set_sequence(Sequence sequence);

    private:
        IntrusiveQueueBase* queue_;
        Sequence            sequence_;
    };

    class IntrusiveQueueBase {
    public:
        using Node     = IntrusiveQueueNode;
        using Sequence = IntrusiveQueueSequence;

        IntrusiveQueueBase();
        ~IntrusiveQueueBase();

        IntrusiveQueueBase(IntrusiveQueueBase&& other);
        IntrusiveQueueBase(const IntrusiveQueueBase&) = delete;
        IntrusiveQueueBase& operator=(IntrusiveQueueBase&& that);
        IntrusiveQueueBase& operator=(const IntrusiveQueueBase&) = delete;

        bool is_empty() const;
        size_t size() const;
        size_t capacity() const;
        size_t tombstone_count() const;
        Sequence push_front(Node& node);
        Sequence push_back(Node& node);
        Node* pop_front();
        Node* pop_back();
        Node* peek_front(size_t relative_offset = 0);
        Node* peek_back(size_t relative_offset = 0);
        void erase(Node& node);
        void erase(Sequence sequence);
        void clear();
        void clear_tombstones();
        void reserve(size_t minimum_capacity);

    private:
        friend class IntrusiveQueueNode;

        void relocated(Node& node);

    private:
        static size_t make_capacity(size_t minimum_capacity);

        static Node** get_slot(Node** slots, Sequence mask, Sequence sequence);
        Node** get_slot(Sequence sequence);

    private:
        // TODO: think about using a BitSet for efficiently clearing tombstones
        //       if we have an erase-heavy workload

        std::unique_ptr<Node*[]> slots_;
        Sequence                 head_;
        Sequence                 tail_;
        Sequence                 mask_;
        size_t                   capacity_;
        size_t                   tombstone_count_;
    };

    // Adds member field support, and hides tombstones.
    template<typename T, IntrusiveQueueNode T::*node_>
    class IntrusiveQueue {
    public:
        using Sequence = IntrusiveQueueSequence;

        IntrusiveQueue() = default;
        explicit IntrusiveQueue(size_t minimum_capacity);

        IntrusiveQueue(IntrusiveQueue&& other) = default;
        IntrusiveQueue(const IntrusiveQueue&) = delete;
        IntrusiveQueue& operator=(IntrusiveQueue&& that) = default;
        IntrusiveQueue& operator=(const IntrusiveQueue&) = delete;

        bool is_empty() const;
        size_t size() const;
        size_t capacity() const;
        Sequence push_front(T& element);
        Sequence push_back(T& element);
        T* pop_front();
        T* pop_back();
        T* peek_front(size_t relative_offset = 0); // to support prefetching
        T* peek_back(size_t relative_offset = 0); // to support prefetching
        void erase(T& element); // replaces the value with a tombstone
        void erase(Sequence sequence); // replaces the value at this sequence with a tombstone
        void clear();
        void reserve(size_t minimum_capacity);

    private:
        static T& from_node(IntrusiveQueueNode& node);
        static T* from_node(IntrusiveQueueNode* node);
        static IntrusiveQueueNode& to_node(T& element);
        static IntrusiveQueueNode* to_node(T* element);

        // Heuristics for deciding if we should clear tombstones.
        bool too_many_tombstones() const;

    private:
        IntrusiveQueueBase base_;
    };


}

#include "intrusive_queue.hpp"
