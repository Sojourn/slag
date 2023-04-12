#include "slag/slag.h"
#include "slag/transform.h"
#include "slag/buffer_writer.h"
#include "slag/message_fragment.h"
#include "slag/message_record_fragment.h"
#include "slag/message_buffer_fragment.h"
#include <vector>
#include <span>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

using namespace slag;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    std::byte buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    BufferWriter writer{buffer};

    MessageRecordFragment fragment;
    fragment.set_head();
    for (size_t i = 0; i < 10; ++i) {
        fragment.push_back(i);
    }
    fragment.set_tail();

    write_message_fragment(writer, fragment);
    asm("int $3");

    return 0;
}
