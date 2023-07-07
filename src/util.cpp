#include "slag/util.h"
#include <stdexcept>
#include <system_error>
#include <cstring>
#include <cerrno>
#include <cassert>

#include <sched.h>

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

    void set_cpu_affinity(int cpu_affinity) {
        cpu_set_t set;

        CPU_ZERO(&set);
        CPU_SET(cpu_affinity, &set);

        if (::sched_setaffinity(0, sizeof(set), &set) < 0) {
            raise_system_error("Failed to set cpu affinity");
        }
    }

}
