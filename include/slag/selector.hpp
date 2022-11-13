#include <cassert>
#include "slag/util.h"

template<typename... Types>
template<typename SelectableType>
inline void slag::Selector<Types...>::insert(SelectableType& selectable, Selectable::EventMask events) {
    constexpr int selectable_type_index = find_type_v<SelectableType, Types...>;
    static_assert(selectable_type_index >= 0);
    static_assert(std::is_base_of_v<Selectable, SelectableType>);

    auto&& [it, inserted] = observers_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(static_cast<Selectable*>(&selectable)),
        std::forward_as_tuple(*this, selectable, selectable_type_index)
    );
    assert(inserted);

    Observer& observer = it->second;
    observer.set_requested_events(events);
    handle_selectable_event(observer, selectable);
}

template<typename... Types>
template<typename SelectableType>
inline void slag::Selector<Types...>::insert(std::unique_ptr<SelectableType> selectable, Selectable::EventMask events) {
    constexpr int selectable_type_index = find_type_v<SelectableType, Types...>;
    static_assert(selectable_type_index >= 0);
    static_assert(std::is_base_of_v<Selectable, SelectableType>);

    auto&& [it, inserted] = observers_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(static_cast<Selectable*>(selectable.get())),
        std::forward_as_tuple(*this, std::move(selectable), selectable_type_index)
    );
    assert(inserted);

    Observer& observer = it->second;
    observer.set_requested_events(events);
    handle_selectable_event(observer, selectable);
}

template<typename... Types>
template<typename SelectableType>
inline void slag::Selector<Types...>::insert(std::shared_ptr<SelectableType> selectable, Selectable::EventMask events) {
    constexpr int selectable_type_index = find_type_v<SelectableType, Types...>;
    static_assert(selectable_type_index >= 0);
    static_assert(std::is_base_of_v<Selectable, SelectableType>);

    auto&& [it, inserted] = observers_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(static_cast<Selectable*>(selectable.get())),
        std::forward_as_tuple(*this, std::move(selectable), selectable_type_index)
    );
    assert(inserted);

    Observer& observer = it->second;
    observer.set_requested_events(events);
    handle_selectable_event(observer, selectable);
}

template<typename... Types>
template<typename SelectableType>
inline void slag::Selector<Types...>::modify(SelectableType& selectable, Selectable::EventMask events) {
    constexpr int selectable_type_index = find_type_v<SelectableType, Types...>;
    static_assert(selectable_type_index >= 0);
    static_assert(std::is_base_of_v<Selectable, SelectableType>);

    if (auto it = observers_.find(static_cast<Selectable*>(&selectable)); it != observers_.end()) {
        Observer& observer = it->second;
        observer.set_requested_events(events);
        handle_selectable_event(observer, selectable); // kick it
    }
    else {
        assert(false);
    }
}

template<typename... Types>
template<typename SelectableType>
inline void slag::Selector<Types...>::erase(SelectableType& selectable) {
    constexpr int selectable_type_index = find_type_v<SelectableType, Types...>;
    static_assert(selectable_type_index >= 0);
    static_assert(std::is_base_of_v<Selectable, SelectableType>);

    if (auto it = observers_.find(static_cast<Selectable*>(&selectable)); it != observers_.end()) {
        Observer& observer = it->second;
        selectable.remove_observer(observer); // prevent destruction callback
        observers_.erase(it);
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

        [&]<int... ints>(std::integer_sequence<int, ints...>) {
            (set_result<ints>(observer, result), ...);
        }(std::make_integer_sequence<int, sizeof...(Types)>());
    }

    return result;
}

template<typename... Types>
inline slag::Selector<Types...>::Observer::Observer(Selector& selector, Selectable& selectable, int selectable_type_index)
    : selector_{selector}
    , selectable_{&selectable}
    , selectable_type_index_{selectable_type_index}
{
}

template<typename... Types>
inline slag::Selector<Types...>::Observer::Observer(Selector& selector, std::unique_ptr<Selectable> selectable, int selectable_type_index)
    : selector_{selector}
    , selectable_{std::move(selectable)}
    , selectable_type_index_{selectable_type_index}
{
}

template<typename... Types>
inline slag::Selector<Types...>::Observer::Observer(Selector& selector, std::shared_ptr<Selectable> selectable, int selectable_type_index)
    : selector_{selector}
    , selectable_{std::move(selectable)}
    , selectable_type_index_{selectable_type_index}
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
inline slag::Selectable& slag::Selector<Types...>::Observer::selectable() {
    // switching on the index probably give better codegen, but is more fragile
    if (auto selectable = std::get_if<Selectable*>(&selectable_)) {
        return **selectable;
    }
    if (auto selectable = std::get_if<std::unique_ptr<Selectable>>(&selectable_)) {
        return *(selectable->get());
    }
    if (auto selectable = std::get_if<std::shared_ptr<Selectable>>(&selectable_)) {
        return *(selectable->get());
    }

    abort();
}

template<typename... Types>
inline const slag::Selectable& slag::Selector<Types...>::Observer::selectable() const {
    return const_cast<Observer>(*this)->selectable();
}

template<typename... Types>
inline int slag::Selector<Types...>::Observer::selectable_type_index() const {
    return selectable_type_index_;
}

template<typename... Types>
inline auto slag::Selector<Types...>::Observer::events() const -> const EventMask& {
    return selector().events();
}

template<typename... Types>
inline auto slag::Selector<Types...>::Observer::requested_events() const -> const EventMask& {
    return requested_events_;
}

template<typename... Types>
inline void slag::Selector<Types...>::Observer::set_requested_events(Selectable::EventMask events) {
    requested_events_ = events;
}

template<typename... Types>
inline void slag::Selector<Types...>::Observer::handle_selectable_event(Selectable& selectable, Selectable::Event event) {
    (void)event;

    selector_.handle_selectable_event(*this, selectable);
}

template<typename... Types>
inline void slag::Selector<Types...>::Observer::handle_selectable_destroyed(Selectable& selectable) {
    selector_.handle_selectable_destroyed(*this, selectable);
}

template<typename... Types>
inline void slag::Selector<Types...>::handle_selectable_event(Observer& observer, Selectable& selectable) {
    (void)selectable;

    if (observer.hook_.is_linked()) {
        return; // already ready
    }
    if ((observer.requested_events() & observer.events()).none()) {
        return; // not a requested event
    }

    ready_observers_.push_back(observer);
}

template<typename... Types>
inline void slag::Selector<Types...>::handle_selectable_destroyed(Observer& observer, Selectable& selectable) {
    (void)observer;

    auto it = observers_.find(&selectable);
    assert(it != observers_.end());
    observers_.erase(it);
}

template<typename... Types>
template<int selectable_type_index>
inline void slag::Selector<Types...>::set_result(Observer& observer, std::optional<std::variant<Types*...>>& result) {
    using TypeTuple = std::tuple<Types...>;

    if (observer.selectable_type_index() == selectable_type_index) {
        assert(!result);
        result = &static_cast<std::tuple_element_t<selectable_type_index, TypeTuple>&>(observer.selectable());
    }
}
