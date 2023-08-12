#include <iostream>

#include "slag/intrusive_queue.h"

using namespace slag;

class Selector;

class Pollable {
    friend class Selector;

public:
    Pollable();
    ~Pollable();

    Pollable(Pollable&& other);
    Pollable(const Pollable&) = delete;
    Pollable& operator=(Pollable&& other);
    Pollable& operator=(const Pollable&) = delete;

private:
    IntrusiveQueueNode selector_hook_;
    bool               is_set_;
};

class Selector : public Pollable {
public:
    void insert(Pollable& pollable) {
        (void)pollable;
    }

    Pollable* select() {
        return nullptr;
    }

private:
    IntrusiveQueue<Pollable, &Pollable::selector_hook_> ready_pollables_;
    IntrusiveQueue<Pollable, &Pollable::selector_hook_> pending_pollables_;
};

struct Foo {
    int value = 3;
    IntrusiveQueueNode node;
};

struct Item {
    int                value;
    IntrusiveQueueNode node1;
    IntrusiveQueueNode node2;

    Item(int value): value(value) {}
};

using Queue1 = IntrusiveQueue<Item, &Item::node1>;
using Queue2 = IntrusiveQueue<Item, &Item::node2>;

int main(int, char**) {
    return 0;
}
