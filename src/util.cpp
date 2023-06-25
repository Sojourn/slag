#include "slag/util.h"
#include <stdexcept>
#include <system_error>
#include <cstring>
#include <cerrno>
#include <cassert>

namespace slag {

    // test tuple_reverse_t
    static_assert(
        std::is_same_v<
            std::tuple<>,
            tuple_reverse_t<std::tuple<>>
        >
    );
    static_assert(
        std::is_same_v<
            std::tuple<int>,
            tuple_reverse_t<std::tuple<int>>
        >
    );
    static_assert(
        std::is_same_v<
            std::tuple<int, int>,
            tuple_reverse_t<std::tuple<int, int>>
        >
    );
    static_assert(
        std::is_same_v<
            std::tuple<int, char, double>,
            tuple_reverse_t<std::tuple<double, char, int>>
        >
    );

    void raise_system_error(const char* message) {
        assert(message);
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category(), message};
    }

}
