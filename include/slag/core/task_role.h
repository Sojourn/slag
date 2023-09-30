#pragma once

#include <string_view>
#include <cstdlib>
#include <cstdint>
#include <cstddef>

#define SLAG_TASK_ROLES(X) \
    X(KNIGHT, "")          \
    X(SQUIRE, "")          \
    X(GENTRY, "")          \
    X(CLERGY, "")          \

namespace slag {

    enum class TaskRole : uint8_t {
#define X(SLAG_TASK_ROLE, SLAG_TASK_ROLE_DESC) \
        SLAG_TASK_ROLE,

        SLAG_TASK_ROLES(X)
#undef X
    };

    constexpr size_t TASK_ROLE_COUNT = 0
#define X(SLAG_TASK_ROLE, SLAG_TASK_ROLE_DESC) \
        + 1                                    \

        SLAG_TASK_ROLES(X)
#undef X
    ;

    constexpr size_t to_index(TaskRole role) {
        return static_cast<size_t>(role);
    }

    constexpr std::string_view to_string_view(TaskRole role) {
        switch (role) {
#define X(SLAG_TASK_ROLE, SLAG_TASK_ROLE_DESC) \
            case TaskRole::SLAG_TASK_ROLE: {   \
                return #SLAG_TASK_ROLE;        \
            }                                  \

            SLAG_TASK_ROLES(X)
#undef X
        }

        abort();
    }

}
