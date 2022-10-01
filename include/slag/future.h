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
        FutureContext();
        FutureContext(FutureContext&&) = delete;
        FutureContext(const FutureContext&) = delete;

        FutureContext& operator=(FutureContext&&) = delete;
        FutureContext& operator=(const FutureContext&) = delete;

        [[nodiscard]] bool is_referenced() const;
        [[nodiscard]] bool is_promise_satisfied() const;
        [[nodiscard]] bool is_future_retrieved() const;
        [[nodiscard]] Event& event();
        [[nodiscard]] const Event& event() const;
        [[nodiscard]] Result<T>& result();
        [[nodiscard]] const Result<T>& result() const;

        void handle_promise_broken();
        void handle_future_retrieved();
        void handle_promise_satisfied();

        void attach(Promise<T>& promise);
        void detach(Promise<T>& promise);

        void attach(Future<T>& future);
        void detach(Future<T>& future);

    private:
        Event     event_;
        bool      promise_attached_  : 1;
        bool      future_attached_   : 1;
        bool      promise_broken_    : 1;
        bool      promise_satisfied_ : 1;
        bool      future_retrieved_  : 1;
        Result<T> result_;
    };

    template<typename T>
    class Promise {
    public:
        Promise();
        Promise(Promise&& other);
        Promise(const Promise&) = delete;
        ~Promise();

        Promise& operator=(Promise&& that);
        Promise& operator=(const Promise&) = delete;

        [[nodiscard]] Future<T> get_future();
        [[nodiscard]] Event& event();
        [[nodiscard]] void set_value(T&& value);
        [[nodiscard]] void set_value(const T& value);
        [[nodiscard]] void set_error(Error error);

        void reset();

    private:
        template<typename U>
        friend class FutureContext;

        template<typename U>
        friend class Future;

        [[nodiscard]] FutureContext<T>& get_context();

    private:
        FutureContext<T>* context_;
    };

    template<typename T>
    class Future {
    public:
        Future();
        Future(Future&& other);
        Future(const Future&) = delete;
        ~Future();

        Future& operator=(Future&& that);
        Future& operator=(const Future&) = delete;

        [[nodiscard]] Event& event();
        [[nodiscard]] const Event& event() const;
        [[nodiscard]] Result<T>& result();
        [[nodiscard]] const Result<T>& result() const;

        void reset();

    private:
        template<typename U>
        friend class FutureContext;

        template<typename U>
        friend class Promise;

        Future(FutureContext<T>& context);

        [[nodiscard]] FutureContext<T>& get_context();
        [[nodiscard]] const FutureContext<T>& get_context() const;

    private:
        FutureContext<T>* context_;
    };

}

#include "future.hpp"
