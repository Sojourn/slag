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

class DummyHandler : public MessageFragmentHandler {
    void handle_message_record_fragment(const MessageRecordFragment& fragment) override {
        (void)fragment;
        // asm("int $3");
    }

    void handle_message_buffer_fragment(const MessageBufferFragment& fragment) override {
        (void)fragment;
        // asm("int $3");
    }
};

class RecordWriter {
public:
    explicit RecordWriter(std::vector<std::byte>& buffer)
        : buffer_{buffer}
        , offset_{buffer.size()}
        , committed_offset_{offset_}
    {
    }

    void begin() {
        assert(offset_ == committed_offset_);
        assert(!fragment_.head());
        assert(!fragment_.tail());
        assert(fragment_.is_empty());

        fragment_.set_head();
    }

    void commit() {
        fragment_.set_tail();
        flush();

        committed_offset_ = offset_;
    }

    void rollback() {
        fragment_.clear();

        offset_ = committed_offset_;
    }

    void write(uint64_t field) {
        if (fragment_.size() == fragment_.capacity()) {
            flush();
        }

        fragment_.push_back(field);
    }

    [[nodiscard]] size_t committed_offset() const {
        return committed_offset_;
    }

private:
    void flush() {
        size_t required_buffer_size_bytes = offset_ + MAX_MESSAGE_RECORD_FRAGMENT_SIZE_BYTES;
        if (buffer_.size() < required_buffer_size_bytes) {
            buffer_.resize(required_buffer_size_bytes);
        }

        BufferWriter writer {
            std::span {
                &buffer_[offset_],
                MAX_MESSAGE_RECORD_FRAGMENT_SIZE_BYTES
            }
        };

        write_message_fragment(writer, fragment_);
        offset_ += writer.offset();
        fragment_.clear();
    }

private:
    std::vector<std::byte>& buffer_;
    size_t                  offset_;
    size_t                  committed_offset_;
    MessageRecordFragment   fragment_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    std::vector<std::byte> buffer;
    RecordWriter writer{buffer};
    writer.begin();
    for (uint64_t i = 0; i < 100; ++i) {
        writer.write(i);
    }
    writer.commit();

    BufferReader reader{std::span{buffer.data(), writer.committed_offset()}};
    DummyHandler handler;
    while (read_message_fragment(reader, handler)) {
        // pass
    }

    std::cout << writer.committed_offset() << std::endl;

    return 0;
}
