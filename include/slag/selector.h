#pragma once

#include <bitset>
#include <memory>
#include <optional>
#include <variant>
#include <unordered_map>
#include <initializer_list>
#include <cstdint>
#include <cstddef>
#include "slag/intrusive_list.h"
#include "slag/pollable.h"
#include "slag/awaitable.h"

namespace slag {

    template<typename... Types>
    class Selector : public Pollable {
    public:
        using Event     = Pollable::Event;
        using EventMask = Pollable::EventMask;

        template<typename PollableType>
        void insert(PollableType& pollable, EventMask events);

        template<typename PollableType>
        void insert(std::unique_ptr<PollableType> pollable, EventMask events);

        template<typename PollableType>
        void insert(std::shared_ptr<PollableType> pollable, EventMask events);

        template<typename PollableType>
        void modify(PollableType& pollable, EventMask events);

        template<typename PollableType>
        void erase(PollableType& pollable);

        [[nodiscard]] std::optional<std::variant<Types*...>> poll();

    private:
        class Observer : public Pollable::Observer {
            friend class Selector<Types...>;

        public:
            Observer(Selector& selector, Pollable& pollable, int pollable_type_index);
            Observer(Selector& selector, std::unique_ptr<Pollable> pollable, int pollable_type_index);
            Observer(Selector& selector, std::shared_ptr<Pollable> pollable, int pollable_type_index);

            [[nodiscard]] Selector& selector();
            [[nodiscard]] const Selector& selector() const;
            [[nodiscard]] Pollable& pollable();
            [[nodiscard]] const Pollable& pollable() const;
            [[nodiscard]] int pollable_type_index() const;
            [[nodiscard]] const EventMask& events() const;
            [[nodiscard]] const EventMask& requested_events() const;
            void set_requested_events(Pollable::EventMask events);

        private:
            void handle_pollable_event(Pollable& pollable, Event event) override;
            void handle_pollable_destroyed(Pollable& pollable) override;

        private:
            using PollableStorage = std::variant<
                Pollable*,
                std::unique_ptr<Pollable>,
                std::shared_ptr<Pollable>
            >;

            Selector&         selector_;
            PollableStorage   pollable_;
            int               pollable_type_index_;
            EventMask         requested_events_;
            IntrusiveListNode ready_hook_;
        };

        void handle_pollable_event(Observer& observer, Pollable& pollable);
        void handle_pollable_destroyed(Observer& observer, Pollable& pollable);

        template<int pollable_type_index, typename Visitor>
        void visit_pollable(Observer& observer, Visitor&& visitor);

        template<typename Visitor>
        void visit_pollable(Observer& observer, Visitor&& visitor);

    private:
        std::unordered_map<Pollable*, Observer>         observers_;
        IntrusiveList<Observer, &Observer::ready_hook_> ready_observers_;
    };

    template<typename... Types>
    class SelectorAwaitable : public Awaitable {
    public:
        SelectorAwaitable(Selector<Types...>& selector)
            : Awaitable{selector, PollableEvent::READABLE}
            , selector_{selector}
        {
        }

        [[nodiscard]] std::variant<Types*...> await_resume() {
            if (auto result = selector_.poll()) {
                return std::move(*result);
            }

            throw std::runtime_error("Pollable destroyed");
        }

    private:
        Selector<Types...>& selector_;
    };

    template<typename... Types>
    [[nodiscard]] inline SelectorAwaitable<Types...> operator co_await(Selector<Types...>& selector) {
        return SelectorAwaitable<Types...>{selector};
    }

}

#include "selector.hpp"
