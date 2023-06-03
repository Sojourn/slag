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
#include "test_generated.h"

using namespace slag;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    TestStruct test_struct;
    test_struct.c = "hello";

    visit_fields(test_struct, []<typename T>(std::string_view name, const T& value) {
        if constexpr (std::is_same_v<T, std::string>) {
            std::cout << name << "=" << value << std::endl;
        }
    });

    return 0;
}
