#pragma once

#include "slag/context.h"
#include "slag/object.h"
#include "reactor.h"
#include "operations/nop_operation.h"
#include "operations/close_operation.h"
#include "operations/poll_multishot_operation.h"
#include "operations/interrupt_operation.h"

namespace slag {

    template<typename... Args>
    inline Ref<NopOperation> start_nop_operation(Args&&... args) {
        Reactor& reactor = get_reactor();

        auto op = reactor.create_operation<NopOperation>(std::forward<Args>(args)...);
        reactor.schedule_operation(*op);
        return op;
    }

    template<typename... Args>
    inline Ref<CloseOperation> start_close_operation(Args&&... args) {
        Reactor& reactor = get_reactor();

        auto op = reactor.create_operation<CloseOperation>(std::forward<Args>(args)...);
        reactor.schedule_operation(*op);
        return op;
    }

    template<typename... Args>
    inline Ref<PollMultishotOperation> start_poll_multishot_operation(Args&&... args) {
        Reactor& reactor = get_reactor();

        auto op = reactor.create_operation<PollMultishotOperation>(std::forward<Args>(args)...);
        reactor.schedule_operation(*op);
        return op;
    }

    template<typename... Args>
    inline Ref<InterruptOperation> start_interrupt_operation(Args&&... args) {
        Reactor& reactor = get_reactor();

        auto op = reactor.create_operation<InterruptOperation>(std::forward<Args>(args)...);
        reactor.schedule_operation(*op);
        return op;
    }

}
