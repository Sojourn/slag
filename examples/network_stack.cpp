#include <iostream>
#include <cstdlib>
#include <cassert>
#include "slag/slag.h"

using namespace slag;

template<typename Stack>
class MessageLayer : public Layer<MessageLayer, Stack> {
    using Base = Layer<MessageLayer, Stack>;

    using Base::above;
    using Base::below;

public:
    MessageLayer() = default;
};

template<typename Stack>
class PacketLayer : public Layer<PacketLayer, Stack> {
    using Base = Layer<PacketLayer, Stack>;

    using Base::above;
    using Base::below;

public:
    PacketLayer() = default;
};

using NetworkStack = Stack<MessageLayer, PacketLayer>;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    NetworkStack stack;

    return EXIT_SUCCESS;
}
