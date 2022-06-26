#include "slag/util.h"
#include <stdexcept>
#include <system_error>
#include <cstring>
#include <cerrno>
#include <cassert>

void slag::raise_system_error(const char* message) {
    assert(message);
    assert(errno != 0);
    throw std::system_error{errno, std::generic_category(), message};
}
