#pragma once

#include <iterator>
#include <cstddef>
#include <cstdint>

namespace slag {

    class IntrusiveListNode {
    public:
        IntrusiveListNode();
        IntrusiveListNode(IntrusiveListNode&& other) noexcept;
        IntrusiveListNode(const IntrusiveListNode&) = delete;
        ~IntrusiveListNode();

        IntrusiveListNode& operator=(IntrusiveListNode&& that) noexcept;
        IntrusiveListNode& operator=(const IntrusiveListNode&) = delete;

        [[nodiscard]] bool is_linked() const;
        void unlink();

    private:
        template<typename T, IntrusiveListNode T::*node_>
        friend class IntrusiveList;

        template<typename T, IntrusiveListNode T::*node_>
        friend class IntrusiveListIterator;

        template<typename T, IntrusiveListNode T::*node_>
        [[nodiscard]] static T& from_node(IntrusiveListNode& node) noexcept;

        template<typename T, IntrusiveListNode T::*node_>
        [[nodiscard]] static IntrusiveListNode& to_node(T& element) noexcept;

        void link_before(IntrusiveListNode& other) noexcept;

    private:
        IntrusiveListNode* prev_;
        IntrusiveListNode* next_;
    };

    // TODO: const_iterator support
    template<typename T, IntrusiveListNode T::*node_>
    class IntrusiveListIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using reference         = value_type&;
        using pointer           = value_type*;

        IntrusiveListIterator();
        explicit IntrusiveListIterator(IntrusiveListNode& node);

        [[nodiscard]] reference operator*();
        [[nodiscard]] pointer operator->();

        // pre-increment/decrement
        IntrusiveListIterator& operator++();
        IntrusiveListIterator& operator--();

        // post-increment/decrement
        IntrusiveListIterator operator++(int);
        IntrusiveListIterator operator--(int);

        [[nodiscard]] bool operator==(const IntrusiveListIterator& that) const;
        [[nodiscard]] bool operator!=(const IntrusiveListIterator& that) const;

    private:
        IntrusiveListNode* current_node_; // because node_ shadows the template parameter
    };

    template<typename T, IntrusiveListNode T::*node_>
    class IntrusiveList {
    public:
        using iterator = IntrusiveListIterator<T, node_>;

        IntrusiveList() = default;
        IntrusiveList(IntrusiveList&& other) noexcept;
        IntrusiveList(const IntrusiveList&) = delete;
        ~IntrusiveList();

        IntrusiveList& operator=(IntrusiveList&& that) noexcept;
        IntrusiveList& operator=(const IntrusiveList&) = delete;

        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] iterator begin();
        [[nodiscard]] iterator end();
        [[nodiscard]] T& front();
        [[nodiscard]] T& back();
        void push_front(T& element);
        void push_back(T& element);
        T& pop_front();
        T& pop_back();
        void erase(T& element);
        iterator erase(iterator it);
        void clear();

    private:
        IntrusiveListNode root_;
    };

}

#include "intrusive_list.hpp"
