#pragma once

#include <bitset>
#include <memory>
#include <optional>
#include <variant>
#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include "slag/intrusive_list.h"
#include "slag/selectable.h"

namespace slag {

    template<typename... Types>
    class Selector : public Selectable {
    public:
        using Event     = Selectable::Event;
        using EventMask = Selectable::EventMask;

        template<typename SelectableType>
        void insert(SelectableType& selectable, EventMask events);

        template<typename SelectableType>
        void insert(std::unique_ptr<SelectableType> selectable, EventMask events);

        template<typename SelectableType>
        void insert(std::shared_ptr<SelectableType> selectable, EventMask events);

        template<typename SelectableType>
        void modify(SelectableType& selectable, EventMask events);

        template<typename SelectableType>
        void erase(SelectableType& selectable);

        [[nodiscard]] std::optional<std::variant<Types*...>> poll();

    private:
        class Observer : public Selectable::Observer {
            friend class Selector<Types...>;

        public:
            Observer(Selector& selector, Selectable& selectable, int selectable_type_index);
            Observer(Selector& selector, std::unique_ptr<Selectable> selectable, int selectable_type_index);
            Observer(Selector& selector, std::shared_ptr<Selectable> selectable, int selectable_type_index);

            [[nodiscard]] Selector& selector();
            [[nodiscard]] const Selector& selector() const;
            [[nodiscard]] Selectable& selectable();
            [[nodiscard]] const Selectable& selectable() const;
            [[nodiscard]] int selectable_type_index() const;
            [[nodiscard]] const EventMask& events() const;
            [[nodiscard]] const EventMask& requested_events() const;
            void set_requested_events(Selectable::EventMask events);

        private:
            void handle_selectable_event(Selectable& selectable, Event event) override;
            void handle_selectable_destroyed(Selectable& selectable) override;

        private:
            using SelectableStorage = std::variant<
                Selectable*,
                std::unique_ptr<Selectable>,
                std::shared_ptr<Selectable>
            >;

            Selector&         selector_;
            SelectableStorage selectable_;
            int               selectable_type_index_;
            EventMask         requested_events_;
            IntrusiveListNode hook_;
        };

        void handle_selectable_event(Observer& observer, Selectable& selectable);
        void handle_selectable_destroyed(Observer& observer, Selectable& selectable);

        template<int selectable_type_index>
        void set_result(Observer& observer, std::optional<std::variant<Types*...>>& result);

    private:
        std::unordered_map<Selectable*, Observer> observers_;
        IntrusiveList<Observer, &Observer::hook_> ready_observers_;
    };

}

#include "selector.hpp"
