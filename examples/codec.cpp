#include "slag/slag.h"
#include "slag/util.h"
#include "slag/visit.h"
#include "slag/transform.h"
#include "slag/buffer_writer.h"
#include "slag/message_fragment.h"
#include "slag/message_record_fragment.h"
#include "slag/message_buffer_fragment.h"
#include "slag/message_encoder.h"
#include <vector>
#include <span>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

using namespace slag;

// JSR: just thinking about how to frame a message
//
// could add a stream offset for the reference message
//
/*
struct MessageHeader {
    uint32_t message_length; // steal bits for flags from here
    uint16_t field_count;
    // uint16_t type;           // this can be a field (small, doesn't change)

    // present bitmask
    // encoding bitmask (less fields that aren't present)
    // encoded fields
    // appendage
};
*/

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    TestStruct test_struct;
    test_struct.a.push_back(0);
    test_struct.a.push_back(1);
    test_struct.a.push_back(2);
    test_struct.c = "hello";

    // std::byte buffer[1024];

    // MessageEncoder encoder;
    // size_t bytes_written = encoder.encode(test_struct, buffer);
    // std::cout << bytes_written << std::endl;

    return 0;
}
