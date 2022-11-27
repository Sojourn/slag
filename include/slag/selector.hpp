#include <cassert>
#include "slag/util.h"

template<typename... Types>
template<typename PollableType>
inline void slag::Selector<Types...>::insert(PollableType& pollable, EventMask events) {
    constexpr int pollable_type_index = find_type_v<PollableType, Types...>;
    static_assert(pollable_type_index >= 0);
    static_assert(std::is_base_of_v<Pollable, PollableType>);

    auto&& [it, inserted] = observers_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(static_cast<Pollable*>(&pollable)),
        std::forward_as_tuple(*this, pollable, pollable_type_index)
    );
    assert(inserted);

    Observer& observer = it->second;
    observer.set_requested_events(events);
    observer.pollable().add_observer(observer);
    handle_pollable_event(observer, pollable);
}

template<typename... Types>
template<typename PollableType>
inline void slag::Selector<Types...>::insert(std::unique_ptr<PollableType> pollable, EventMask events) {
    constexpr int pollable_type_index = find_type_v<PollableType, Types...>;
    static_assert(pollable_type_index >= 0);
    static_assert(std::is_base_of_v<Pollable, PollableType>);

    auto&& [it, inserted] = observers_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(static_cast<Pollable*>(pollable.get())),
        std::forward_as_tuple(*this, std::move(pollable), pollable_type_index)
    );
    assert(inserted);

    Observer& observer = it->second;
    observer.set_requested_events(events);
    observer.pollable().add_observer(observer);
    handle_pollable_event(observer, pollable);
}

template<typename... Types>
template<typename PollableType>
inline void slag::Selector<Types...>::insert(std::shared_ptr<PollableType> pollable, EventMask events) {
    constexpr int pollable_type_index = find_type_v<PollableType, Types...>;
    static_assert(pollable_type_index >= 0);
    static_assert(std::is_base_of_v<Pollable, PollableType>);

    auto&& [it, inserted] = observers_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(static_cast<Pollable*>(pollable.get())),
        std::forward_as_tuple(*this, std::move(pollable), pollable_type_index)
    );
    assert(inserted);

    Observer& observer = it->second;
    observer.set_requested_events(events);
    observer.pollable().add_observer(observer);
    handle_pollable_event(observer, pollable);
}

template<typename... Types>
template<typename PollableType>
inline void slag::Selector<Types...>::modify(PollableType& pollable, EventMask events) {
    constexpr int pollable_type_index = find_type_v<PollableType, Types...>;
    static_assert(pollable_type_index >= 0);
    static_assert(std::is_base_of_v<Pollable, PollableType>);

    if (auto it = observers_.find(static_cast<Pollable*>(&pollable)); it != observers_.end()) {
        Observer& observer = it->second;
        observer.set_requested_events(events);
        handle_pollable_event(observer, pollable); // kick it
    }
    else {
        assert(false);
    }
}

template<typename... Types>
template<typename PollableType>
inline void slag::Selector<Types...>::erase(PollableType& pollable) {
    constexpr int pollable_type_index = find_type_v<PollableType, Types...>;
    static_assert(pollable_type_index >= 0);
    static_assert(std::is_base_of_v<Pollable, PollableType>);

    if (auto it = observers_.find(static_cast<Pollable*>(&pollable)); it != observers_.end()) {
        Observer& observer = it->second;
        pollable.remove_observer(observer); // prevent destruction callback
        observers_.erase(it);
        if (ready_observers_.is_empty()) {
            set_event(Event::READABLE, false);
        }
    }
    else {
        assert(false);
    }
}

template<typename... Types>
inline std::optional<std::variant<Types*...>> slag::Selector<Types...>::poll() {
    std::optional<std::variant<Types*...>> result;
    if (!ready_observers_.is_empty()) {
        Observer& observer = ready_observers_.pop_front();
        visit_pollable(observer, [&](auto&& pollable) {
            result = &pollable;
        });

        if (ready_observers_.is_empty()) {
            set_event(Event::READABLE, false);
        }
    }

    return result;
}

template<typename... Types>
inline slag::Selector<Types...>::Observer::Observer(Selector& selector, Pollable& pollable, int pollable_type_index)
    : selector_{selector}
    , pollable_{&pollable}
    , pollable_type_index_{pollable_type_index}
{
}

template<typename... Types>
inline slag::Selector<Types...>::Observer::Observer(Selector& selector, std::unique_ptr<Pollable> pollable, int pollable_type_index)
    : selector_{selector}
    , pollable_{std::move(pollable)}
    , pollable_type_index_{pollable_type_index}
{
}

template<typename... Types>
inline slag::Selector<Types...>::Observer::Observer(Selector& selector, std::shared_ptr<Pollable> pollable, int pollable_type_index)
    : selector_{selector}
    , pollable_{std::move(pollable)}
    , pollable_type_index_{pollable_type_index}
{
}

template<typename... Types>
inline slag::Selector<Types...>& slag::Selector<Types...>::Observer::selector() {
    return selector_;
}

template<typename... Types>
inline const slag::Selector<Types...>& slag::Selector<Types...>::Observer::selector() const {
    return selector_;
}

template<typename... Types>
inline slag::Pollable& slag::Selector<Types...>::Observer::pollable() {
    // switching on the index probably give better codegen, but is more fragile
    if (auto pollable = std::get_if<Pollable*>(&pollable_)) {
        return **pollable;
    }
    if (auto pollable = std::get_if<std::unique_ptr<Pollable>>(&pollable_)) {
        return *(pollable->get());
    }
    if (auto pollable = std::get_if<std::shared_ptr<Pollable>>(&pollable_)) {
        return *(pollable->get());
    }

    abort();
}

template<typename... Types>
inline const slag::Pollable& slag::Selector<Types...>::Observer::pollable() const {
    return const_cast<Observer>(*this)->pollable();
}

template<typename... Types>
inline int slag::Selector<Types...>::Observer::pollable_type_index() const {
    return pollable_type_index_;
}

template<typename... Types>
inline auto slag::Selector<Types...>::Observer::events() const -> const EventMask& {
    return pollable().events();
}

template<typename... Types>
inline auto slag::Selector<Types...>::Observer::requested_events() const -> const EventMask& {
    return requested_events_;
}

template<typename... Types>
inline void slag::Selector<Types...>::Observer::set_requested_events(EventMask events) {
    requested_events_ = events;
}

template<typename... Types>
inline void slag::Selector<Types...>::Observer::handle_pollable_event(Pollable& pollable, Event event) {
    (void)event;

    selector_.handle_pollable_event(*this, pollable);
}

template<typename... Types>
inline void slag::Selector<Types...>::Observer::handle_pollable_destroyed(Pollable& pollable) {
    selector_.handle_pollable_destroyed(*this, pollable);
}

template<typename... Types>
inline void slag::Selector<Types...>::handle_pollable_event(Observer& observer, Pollable& pollable) {
    if (observer.ready_hook_.is_linked()) {
        return; // already ready
    }
    if ((observer.requested_events() & pollable.events()).none()) {
        return; // not a requested event
    }

    ready_observers_.push_back(observer);

    // the selector itself is now readable
    set_event(Event::READABLE, true);
}

template<typename... Types>
inline void slag::Selector<Types...>::handle_pollable_destroyed(Observer& observer, Pollable& pollable) {
    (void)observer;

    auto it = observers_.find(&pollable);
    assert(it != observers_.end());
    observers_.erase(it);

    if (ready_observers_.is_empty()) {
        set_event(Event::READABLE, false);
    }
}

template<typename... Types>
template<int pollable_type_index, typename Visitor>
inline void slag::Selector<Types...>::visit_pollable(Observer& observer, Visitor&& visitor) {
    using TypeTuple = std::tuple<Types...>;
    using Type      = std::tuple_element_t<pollable_type_index, TypeTuple>;

    if (observer.pollable_type_index() == pollable_type_index) {
        visitor(static_cast<Type&>(observer.pollable()));
    }
}

template<typename... Types>
template<typename Visitor>
inline void slag::Selector<Types...>::visit_pollable(Observer& observer, Visitor&& visitor) {
    [&]<int... ints>(std::integer_sequence<int, ints...>) {
        (visit_pollable<ints>(observer, visitor), ...);
    }(std::make_integer_sequence<int, sizeof...(Types)>());
}
