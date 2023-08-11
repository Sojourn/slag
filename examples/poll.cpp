#include <iostream>

#include "slag/intrusive_queue.h"

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

int main(int argc, char** argv) {
    return 0;
}
