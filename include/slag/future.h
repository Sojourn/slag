#pragma once

#include <utility>
#include <variant>
#include <type_traits>
#include <stdexcept>
#include <cassert>
#include "slag/error.h"
#include "slag/event.h"

namespace slag {

    template<typename T>
    class Future;

    template<typename T>
    class Promise;

    template<typename T>
    class FutureContext {
    public:
        static_assert(!std::is_same_v<T, std::exception_ptr>);

        using Result = std::variant<
            std::monostate,
            std::conditional_t<std::is_same_v<T, void>, int, T>,
            std::exception_ptr
        >;

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
        [[nodiscard]] Result& result();
        [[nodiscard]] const Result& result() const;

        void handle_promise_broken();
        void handle_future_retrieved();
        void handle_promise_satisfied();

        // TODO: handle moves?
        void attach(Promise<T>& promise);
        void detach(Promise<T>& promise);

        // TODO: handle moves?
        void attach(Future<T>& future);
        void detach(Future<T>& future);

    private:
        Event  event_;
        bool   promise_attached_  : 1;
        bool   future_attached_   : 1;
        bool   promise_broken_    : 1;
        bool   promise_satisfied_ : 1;
        bool   future_retrieved_  : 1;
        Result result_;
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
        [[nodiscard]] void set_error(Error error, std::string_view message);
        [[nodiscard]] void set_exception(std::exception_ptr ex);
        [[nodiscard]] void set_current_exception();

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

        [[nodiscard]] bool is_ready() const;
        [[nodiscard]] Event& event();
        [[nodiscard]] const Event& event() const;
        [[nodiscard]] T& get();
        [[nodiscard]] const T& get() const;

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

    template<>
    class Future<void> {
    public:
        Future()
            : context_{nullptr}
        {
        }

        Future(Future&& other)
            : context_{std::exchange(other.context_, nullptr)}
        {
        }

        Future(const Future&) = delete;

        ~Future() {
            reset();
        }

        Future& operator=(Future&& that) {
            if (this != &that) {
                reset();
                context_ = std::exchange(that.context_, nullptr);
            }

            return *this;
        }

        Future& operator=(const Future&) = delete;

        template<typename T>
        [[nodiscard]] bool is_ready() const {
            return get_context().is_promise_satisfied();
        }

        [[nodiscard]] Event& event() {
            return get_context().event();
        }

        [[nodiscard]] const Event& event() const {
            return get_context().event();
        }

        [[nodiscard]] void get() const {
            auto&& context = get_context();
            if (!context.is_promise_satisfied()) {
                Error{ErrorCode::FUTURE_NOT_READY}.raise("Failed to get result");
            }

            auto&& result = context.result();
            if (auto ex = std::get_if<std::exception_ptr>(&result)) {
                std::rethrow_exception(*ex);
            }
        }

        void reset() {
            if (context_) {
                context_->detach(*this);
                if (!context_->is_referenced()) {
                    delete context_;
                }

                context_ = nullptr;
            }
        }

    private:
        template<typename U>
        friend class FutureContext;

        template<typename U>
        friend class Promise;

        Future(FutureContext<void>& context)
            : context_{&context}
        {
            context_->attach(*this);
        }

        [[nodiscard]] FutureContext<void>& get_context() {
            if (!context_) {
                Error{ErrorCode::FUTURE_DETACHED}.raise("Failed to get the future context");
            }

            return *context_;
        }

        [[nodiscard]] const FutureContext<void>& get_context() const {
            if (!context_) {
                Error{ErrorCode::FUTURE_DETACHED}.raise("Failed to get the future context");
            }

            return *context_;
        }

    private:
        FutureContext<void>* context_;
    };

    template<>
    class Promise<void> {
    public:
        Promise()
            : context_{nullptr}
        {
        }

        Promise(Promise&& other)
            : context_{std::exchange(other.context_, nullptr)}
        {
        }

        Promise(const Promise&) = delete;

        ~Promise() {
            reset();
        }

        Promise& operator=(Promise&& that) {
            if (this != &that) {
                reset();
                context_ = std::exchange(that.context_, nullptr);
            }

            return *this;
        }

        Promise& operator=(const Promise&) = delete;

        [[nodiscard]] Future<void> get_future() {
            return Future<void>{get_context()};
        }

        [[nodiscard]] Event& event() {
            return get_context().event();
        }

        [[nodiscard]] void set_value() {
            FutureContext<void>& context = get_context();
            if (context.is_promise_satisfied()) {
                Error{ErrorCode::PROMISE_ALREADY_SATISFIED}.raise("Failed to set future value");
            }

            context.result() = int{1};
            context.handle_promise_satisfied();
        }

        [[nodiscard]] void set_error(Error error, std::string_view message) {
            try {
                error.raise(message);
            }
            catch (const std::exception&) {
                set_current_exception();
            }
        }

        [[nodiscard]] void set_exception(std::exception_ptr ex) {
            FutureContext<void>& context = get_context();
            if (context.is_promise_satisfied()) {
                Error{ErrorCode::PROMISE_ALREADY_SATISFIED}.raise("Failed to set future error");
            }

            context.result() = ex;
            context.handle_promise_satisfied();
        }

        [[nodiscard]] void set_current_exception() {
            set_exception(std::current_exception());
        }

        void reset() {
            if (context_) {
                context_->detach(*this);
                if (!context_->is_referenced()) {
                    delete context_;
                }

                context_ = nullptr;
            }
        }

    private:
        template<typename U>
        friend class FutureContext;

        template<typename U>
        friend class Future;

        [[nodiscard]] FutureContext<void>& get_context() {
            if (!context_) {
                context_ = new FutureContext<void>; // TODO: use custom allocator(s)
                context_->attach(*this);
            }

            return *context_;
        }

    private:
        FutureContext<void>* context_;
    };

}

#include "future.hpp"
