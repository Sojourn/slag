#include "slag/slag.h"
#include "slag/util.h"
#include "slag/visit.h"
#include "slag/transform.h"
#include "slag/buffer_writer.h"
#include "slag/message_fragment.h"
#include "slag/message_record_fragment.h"
#include "slag/message_buffer_fragment.h"
#include "slag/message_encoder.h"
#include "slag/stack.h"
#include "slag/stack.h"
#include <vector>
#include <span>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

using namespace slag;

template<typename Stack>
struct RecordLayer : Layer<RecordLayer, Stack> {
    using Layer<RecordLayer, Stack>::above;
    using Layer<RecordLayer, Stack>::below;

    void start() {
        descend();
    }

    void ascend() {
        std::cout << "RecordLayer::ascend" << std::endl;
        // above()->ascend();
    }

    void descend() {
        std::cout << "RecordLayer::descend" << std::endl;
        below()->descend();
    }
};

template<typename Stack>
struct MessageLayer : Layer<MessageLayer, Stack> {
    using Layer<MessageLayer, Stack>::above;
    using Layer<MessageLayer, Stack>::below;

    void start() {
    }

    void ascend() {
        std::cout << "MessageLayer::ascend" << std::endl;
        above()->ascend();
    }

    void descend() {
        std::cout << "MessageLayer::descend" << std::endl;
        below()->descend();
    }
};

template<typename Stack>
struct TcpLayer : Layer<TcpLayer, Stack> {
    using Layer<TcpLayer, Stack>::above;
    using Layer<TcpLayer, Stack>::below;

    void start() {
        ascend();
    }

    void ascend() {
        std::cout << "TcpLayer::ascend" << std::endl;
        above()->ascend();
    }

    void descend() {
        std::cout << "TcpLayer::descend" << std::endl;
        // below()->descend();
    }
};

using NetworkStack = Stack <
    RecordLayer,
    MessageLayer,
    TcpLayer
>;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    NetworkStack stack;

    // TestStruct src_record;
    // src_record.a.push_back(0);
    // src_record.a.push_back(1);
    // src_record.a.push_back(2);
    // src_record.c = "hello";

    // Message message;
    // MessageWriter writer{message};
    // MessageReader reader{message};

    // encode(src_record, writer);
    // {
    //     TestStruct dst_record;
    //     decode(dst_record, reader);
    //     asm("int $3");
    // }

    return 0;
}
