#pragma once

namespace slag {

    // A basic intrusive list implementation until I bite the
    // bullet and do a proper one.

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

        void link_before(IntrusiveListNode& other);

    private:
        IntrusiveListNode* prev_;
        IntrusiveListNode* next_;
    };

    template<typename T, IntrusiveListNode T::*node_>
    class IntrusiveList {
    public:
        IntrusiveList() = default;
        IntrusiveList(IntrusiveList&& other) noexcept;
        IntrusiveList(const IntrusiveList&) = delete;
        ~IntrusiveList();

        IntrusiveList& operator=(IntrusiveList&& that) noexcept;
        IntrusiveList& operator=(const IntrusiveList&) = delete;

        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] T& front();
        [[nodiscard]] T& back();
        void push_front(T& element);
        void push_back(T& element);
        T& pop_front();
        T& pop_back();
        void erase(T& element);
        void clear();

    private:
        [[nodiscard]] static T& from_node(IntrusiveListNode& node) noexcept;
        [[nodiscard]] static IntrusiveListNode& to_node(T& element) noexcept;

    private:
        IntrusiveListNode root_;
    };

}

#include "intrusive_list.hpp"
