#include <iostream>
#include "slag/slag.h"

using namespace slag;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    auto&& [future, promise] = make_future<int>();

    std::cout << future.get() << std::endl;
    promise.set_value(13);

    return 0;
}
