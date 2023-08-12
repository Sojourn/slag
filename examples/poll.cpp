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
    Selector* selector_;
    bool      set_;
};

class Selector : public Pollable {
public:
    void insert();
    void remove();
    Pollable* select();

private:
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
