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

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    TestStruct src_record;
    src_record.a.push_back(0);
    src_record.a.push_back(1);
    src_record.a.push_back(2);
    src_record.c = "hello";

    Message message;
    MessageWriter writer{message};
    MessageReader reader{message};

    encode(src_record, writer);
    {
        TestStruct dst_record;
        decode(dst_record, reader);
        asm("int $3");
    }

    return 0;
}
