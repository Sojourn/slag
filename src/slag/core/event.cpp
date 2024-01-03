#include "event.h"
#include "selector.h"
#include <cassert>

namespace slag {

    Event::Event()
        : user_data_{nullptr}
        , selector_{nullptr}
        , is_set_{false}
    {
    }

    Event::Event(Event&& other)
        : Event{}
    {
        std::swap(user_data_, other.user_data_);
        std::swap(selector_, other.selector_);
        std::swap(selector_hook_, other.selector_hook_);
        std::swap(is_set_, other.is_set_);
    }

    Event& Event::operator=(Event&& that) {
        if (this != &that) {
            selector_ = nullptr;
            selector_hook_.unlink();

            std::swap(user_data_, that.user_data_);
            std::swap(selector_, that.selector_);
            std::swap(selector_hook_, that.selector_hook_);
            std::swap(is_set_, that.is_set_);
        }

        return *this;
    }

    Event::operator bool() const {
        return is_set();
    }

    bool Event::is_set() const {
        return is_set_;
    }

    void Event::set(bool value) {
        if (is_set_ == value) {
            return; // no-op
        }

        is_set_ = value;

        if (selector_) {
            selector_->handle_readiness_change(*this);
        }
    }

    void Event::reset() {
        set(false);
    }

    void* Event::user_data() {
        return user_data_;
    }

    const void* Event::user_data() const {
        return user_data_;
    }

    void Event::set_user_data(void* user_data) {
        user_data_ = user_data;
    }

    bool Event::is_linked() const {
        return static_cast<bool>(selector_);
    }

    void Event::unlink() {
        selector_ = nullptr;
        selector_hook_.unlink();
    }

    void Event::attach(Selector& selector) {
        assert(selector_ == nullptr);
        assert(!selector_hook_.is_linked());

        selector_ = &selector;
    }

    void Event::detach(Selector& selector) {
        assert(selector_ == &selector);
        assert(!selector_hook_.is_linked());

        selector_ = nullptr;
    }

}
