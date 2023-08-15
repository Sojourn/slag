#include <iostream>
#include <span>
#include <bitset>

#include "slag/intrusive_queue.h"

using namespace slag;

#if 0

class Pollable;
class Selector;

enum class EventType {
    READABLE,
    WRITABLE,
    POLLABLE,
    RUNNABLE,
    COMPLETE,
    CANCELED,
};

template<EventType event_type>
class Awaitable;

template<>
class Awaitable<EventType::READABLE> {
protected:
    ~Awaitable() = default;

public:
    // Interfaces need a unique function name to prevent
    // things from getting weird.
    virtual Event& is_readable() = 0;
};

// Helper functions to treat these generically
template<EventType event_type>
Event& get_event(Awaitable<event_type>& awaitable) {
    if constexpr (event_type == EventType::READABLE) {
        return awaitable.readable_event();
    }
    // ...

    abort();
}

class Event {
public:
    Event();
    Event(Event&& other);
    Event(const Event&) = delete;

    Event& operator=(Event&& other);
    Event& operator=(const Event&) = delete;

    bool is_set() const;
    void set(bool value = true);
    void reset();

    void* user_data();
    const void* user_data() const;
    void set_user_data(void* user_data);

private:
    friend class Selector;

    void attach(Selector& selector);
    void detach(Selector& selector);

private:
    void*              user_data_;
    Selector*          selector_;
    IntrusiveQueueNode selector_hook_;
    bool               is_ready_;
};

// Transform events (rate limiting)

class Selector : public Pollable {
    friend class Event;

public:
    void insert(Event& event);

    template<size_t EXTENT>
    size_t select(std::span<Event*, EXTENT> events);

    void remove(Event& event);
    Event* select();

    Event& pollable_event() override;
};

class Selector {
    friend class Pollable;

public:
    Selector() = default;

    ~Selector() {
        // The lifetime of pollables we are tracking should not exceed our own.
        // This is checking that pollables have been destructed and removed themselves
        // from the ready queue, not that the ready queue has been exhausted.
        assert(ready_queue_.is_empty());
    }

    void insert(std::span<Pollable*> pollables) {
        for (Pollable* pollable: pollables) {
            if (!pollable) {
                continue; // The pollables array can be sparse.
            }

            pollable->attach(*this);
        }

        update_readiness();
    }

    void insert(Pollable& pollable) {
        Pollable* pointer = &pollable;
        insert(std::span{&pointer, 1});
    }

    void remove(std::span<Pollable*> pollables) {
        for (Pollable* pollable: pollables) {
            if (!pollable) {
                return; // The pollables array can be sparse.
            }

            // The pollable cannot be linked when it is detached.
            if (pollable->is_set()) {
                ready_queue_.erase(*pollable);
            }

            // Disassociate it from us.
            pollable->detach(*this);
        }

        // Update our own readiness in case we drained the ready queue.
        update_readiness();
    }

    void remove(Pollable& pollable) {
        Pollable* pointer = &pollable;
        remove(std::span{&pointer, 1});
    }

    // NOTE: pollables will not be selected again until they are re-inserted.
    size_t select(std::span<Pollable*> pollables) {
        // Take pollables from the ready queue.
        size_t count = ready_queue_.pop_front(pollables);

        // Detach them as an easy way of getting edge-triggered behavior.
        for (size_t index = 0; index < count; ++index) {
            pollables[index]->detach(*this);
        }

        // Update our own readiness in case we exhausted the ready queue.
        update_readiness();

        return count;
    }

    // NOTE: pollables will not be selected again until they are re-inserted.
    // NOTE: this will attempt to prefetch the next ready pollable.
    Pollable* select() {
        Pollable* pollable = ready_queue_.pop_front();
        if (pollable) {
            pollable->detach(*this);
            update_readiness();
        }

        return nullptr;
    }

private:
    friend class Pollable;

    // Handle a readiness change of one of the pollables we are tracking.
    void handle_readiness_change(Pollable& pollable) {
        if (pollable.is_set()) {
            ready_queue_.push_back(pollable);
        }
        else {
            ready_queue_.erase(pollable);
        }
    }

    // Update our own readiness. We are considered ready as long as
    // our internal ready queue is not empty.
    void update_readiness() {
        set(!ready_queue_.is_empty());
    }

private:
    IntrusiveQueue<Pollable, &Pollable::selector_hook_> ready_queue_;
};

Pollable::Pollable()
    : user_data_{nullptr}
    , selector_{nullptr}
    , is_ready_{false}
{
}

Pollable::Pollable(Pollable&& other)
    : Pollable{}
{
    std::swap(user_data_, other.user_data_);
    std::swap(selector_, other.selector_);
    std::swap(selector_hook_, other.selector_hook_);
    std::swap(is_ready_, other.is_ready_);
}

Pollable& Pollable::operator=(Pollable&& that) {
    if (this != &that) {
        selector_ = nullptr;
        selector_hook_.unlink();

        std::swap(user_data_, that.user_data_);
        std::swap(selector_, that.selector_);
        std::swap(selector_hook_, that.selector_hook_);
        std::swap(is_ready_, that.is_ready_);
    }

    return *this;
}

bool Pollable::is_set() const {
    return is_ready_;
}

void Pollable::set(bool value) {
    if (is_ready_ == value) {
        return; // no-op
    }

    is_ready_ = value;

    if (selector_) {
        selector_->handle_readiness_change(*this);
    }
}

void* Pollable::user_data() {
    return user_data_;
}

const void* Pollable::user_data() const {
    return user_data_;
}

void Pollable::set_user_data(void* user_data) {
    user_data_ = user_data;
}

void Pollable::attach(Selector& selector) {
    assert(!selector_);
    assert(!selector_hook_.is_linked());

    selector_ = &selector;

    // Immediately ready.
    if (is_ready_) {
        selector_->handle_readiness_change(*this);
    }
}

void Pollable::detach(Selector& selector) {
    assert(selector_ == &selector);
    assert(!selector_hook_.is_linked());

    selector_ = nullptr;
}

class Task {
public:
    virtual ~Task() = default;
    virtual Pollable& runnable() = 0;
    virtual void run() = 0;
};

class Event;

// Interfaces
class Pollable {
protected:
    ~Pollable() = default;

public:
    virtual Event& pollable_event() = 0;
};

class Readable {
    virtual Event& readable_event() = 0;
};

class Runnable {
    virtual Event& runnable_event() = 0;
};

class Executor : public Runnable {
public:
    void insert(Runnable& runnable);
    void remove(Runnable& runnable);

    Event& runnable_event() override {
        return selector_.pollable_event();
    }

private:
    friend class Task;

    void attach(Task& task) {
    }

    void detach(Task& task) {
        selector_.remove(task);
    }

private:
    Selector selector_;
};

#endif

int main(int, char**) {
    return 0;
}
