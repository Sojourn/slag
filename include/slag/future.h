#pragma once

#include "slag/error.h"
#include "slag/result.h"
#include "slag/event.h"

namespace slag {

    template<typename T>
    class Future;

    template<typename T>
    class Promise;

    template<typename T>
    class FutureContext {
    public:
        FutureContext(Promise<T>& promise);

        // solves needing a void specialization, but might cause extra moves/copies
        void set_result(Result<T> result);

        void attach(Promise<T>& promise);
        void detach(Promise<T>& promise);

        void attach(Future<T>& future);
        void detach(Future<T>& future);

    private:
        Future<T>*  future_;
        Promise<T>* promise_;
        Event       future_event_;
        Event       promise_event_;
        Result<T>   result_;
        bool        promise_broken_;
        bool        promise_satisfied_;
        bool        future_detached_;
        bool        future_abandoned_;
    };

    template<typename T>
    class Future {
    public:
    };

    template<typename T>
    class Promise {
    public:
    };

}
