#pragma once

#include <optional>
#include <iterator>
#include <array>
#include <memory>
#include <cstdint>
#include <cstddef>

namespace slag {

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

        [[nodiscard]] bool is_linked() const;
        [[nodiscard]] std::optional<Sequence> sequence() const;
        void unlink();

    private:
        friend class IntrusiveQueueBase;

        void attach(IntrusiveQueueBase& queue, Sequence sequence);
        void detach(IntrusiveQueueBase& queue);

        void set_queue(IntrusiveQueueBase& queue);
        void set_sequence(Sequence sequence);

    private:
        IntrusiveQueueBase* queue_;
        Sequence            sequence_;
    };

    // TODO: need to attach/detach the nodes
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
        Sequence push_front(IntrusiveQueueNode& node);
        Sequence push_back(IntrusiveQueueNode& node);
        IntrusiveQueueNode* pop_front();
        IntrusiveQueueNode* pop_back();
        IntrusiveQueueNode* peek_front(size_t relative_offset);
        IntrusiveQueueNode* peek_back(size_t relative_offset);
        void erase(IntrusiveQueueNode& node);
        void erase(Sequence sequence);
        void clear();
        void clear_tombstones();
        void reserve(size_t minimum_capacity);

    private:
        friend class IntrusiveQueueNode;

        void relocated(IntrusiveQueueNode& node);

    private:
        static size_t make_capacity(size_t minimum_capacity);

        Slot& get_slot(Sequence sequence);
        Slot& get_slot(Slot* slots, Sequence sequence);

    private:
        // TODO: think about using a BitSet for efficiently clearing tombstones
        //       if we have an erase-heavy workload

        std::unique_ptr<Slot[]> slots_;
        Sequence                head_;
        Sequence                tail_;
        Sequence                mask_;
        size_t                  capacity_;
        size_t                  tombstone_count_;
    };

    // SwissQueue, since it can be full of holes?
    template<typename T, IntrusiveQueueNode T::*node_>
    class IntrusiveQueue {
        // TODO: ensure T subclasses IntrusiveQueueNode

    public:
        using Sequence = IntrusiveQueueSequence;

        explicit IntrusiveQueue(size_t minimum_capacity = 16);

        IntrusiveQueue(IntrusiveQueue&& other) = default;
        IntrusiveQueue(const IntrusiveQueue&) = delete;
        IntrusiveQueue& operator=(IntrusiveQueue&& that) = default;
        IntrusiveQueue& operator=(const IntrusiveQueue&) = delete;

        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] size_t size() const;
        [[nodiscard]] size_t capacity() const;
        [[nodiscard]] size_t tombstone_count() const;
        Sequence push_front(T& element);
        Sequence push_back(T& element);
        [[nodiscard]] T* pop_front();
        [[nodiscard]] T* pop_back();
        [[nodiscard]] T* peek_front(size_t relative_offset = 0); // to support prefetching
        [[nodiscard]] T* peek_back(size_t relative_offset = 0); // to support prefetching
        void erase(T& element); // replaces the value with a tombstone
        void erase(Sequence sequence); // replaces the value at this sequence with a tombstone
        void clear();
        void clear_tombstones();
        void reserve(size_t minimum_capacity);

    private:
        static T& from_node(IntrusiveQueueNode& node);
        static T* from_node(IntrusiveQueueNode* node);
        static IntrusiveQueueNode& to_node(T& element);
        static IntrusiveQueueNode* to_node(T* element);

    private:
        IntrusiveQueueBase base_;
    };


}

#include "intrusive_queue.hpp"
