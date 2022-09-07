#include <iostream>
#include <any>
#include <span>
#include <memory>
#include <vector>
#include <functional>
#include "slag/slag.h"

using namespace slag;

template<typename ReturnType>
class Coroutine {
public:
    template<typename Functor>
    Coroutine(Functor&& functor)
        : functor_{std::forward<Functor>(functor)}
    {
    }

    [[nodiscard]] bool run(Task& task) {
        Result<ReturnType> intermediate_result = functor_(task);
        if (intermediate_result) {
            // TODO: use type_traits to avoid pessimized moves
            result_.set_value(std::move(intermediate_result.value()));
            return true;
        }
        else if (intermediate_result.error() == ErrorCode::TRY_AGAIN_LATER) {
            return false;
        }
        else {
            result_.set_error(intermediate_result.error());
            return true;
        }
    }

    [[nodiscard]] Future<ReturnType> get_future() {
        return result_.get_future();
    }

private:
    Promise<ReturnType>                      result_;
    std::function<Result<ReturnType>(Task&)> functor_;
};

template<typename ReturnType>
class CoroutineTask : public Task {
public:
    template<typename Functor>
    CoroutineTask(Functor&& functor);

private:
    Coroutine<ReturnType> coroutine_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    return 0;
}
