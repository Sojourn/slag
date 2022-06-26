#pragma once

namespace slag {

    // raise a system error for the currently set errno
    void raise_system_error(const char* message);

}
