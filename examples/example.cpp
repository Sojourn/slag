#include <iostream>
#include <any>
#include <span>
#include <memory>
#include <vector>
#include "slag/slag.h"

using namespace slag;
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Future<int> future;
    Promise<int> promise;

    future = promise.get_future();

    try {
        future.result().error().raise("foo");
    } catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    promise.set_value(3);

    std::cout << future.result().value() << std::endl;

    return 0;
}
