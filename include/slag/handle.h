#pragma once

#include <utility>
#include <memory>

namespace slag {

    // TODO: replace with a non-atomic table based version
    template<typename T>
    using Handle = std::shared_ptr<T>;

    template<typename T, typename... Args>
    [[nodiscard]] inline Handle<T> make_handle(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

}
