#pragma once

#include <limits>
#include "slag/postal/buffer.h"
#include "slag/postal/buffer_cursor.h"

namespace slag::postal {

    class BufferSlice {
    public:
        BufferSlice(BufferHandle buffer, BufferCursor first, BufferCursor last);

        explicit operator bool() const;
        bool is_empty() const;
        size_t size() const;

        BufferHandle& buffer();
        BufferCursor begin();
        BufferCursor end();

        BufferSlice first(size_t size);
        BufferSlice middle(size_t relative_offset, size_t size = std::numeric_limits<size_t>::max());
        BufferSlice last(size_t size);

        template<typename T>
        bool load(T& value) const;

        template<typename T>
        bool store(const T& value);

    private:
        BufferHandle buffer_;
        BufferCursor first_;
        BufferCursor last_;
    };

    template<typename T>
    bool BufferSlice::load(T& value) const {
        if (size() < sizeof(value)) {
            return false;
        }

        std::byte* dst_beg = reinterpret_cast<std::byte*>(&value);
        std::byte* dst_pos = dst_beg;
        std::byte* dst_end = dst_beg + sizeof(value);

        BufferCursor cursor = first_;

        while (dst_pos < dst_end) {
            std::span<std::byte> src_buf = cursor.content();

            size_t cnt = std::min(dst_end - dst_pos, src_buf.size_bytes());
            memcpy(dst_pos, src_buf.data(), cnt);

            size_t adv_cnt = cursor.advance(cnt);
            assert(adv_cnt == cnt);
            dst_pos += cnt;
        }

        return true;
    }

    template<typename T>
    bool BufferSlice::store(const T& value) {
        if (size() < sizeof(value)) {
            return false;
        }

        const std::byte* src_beg = reinterpret_cast<const std::byte*>(&value);
        const std::byte* dst_pos = src_beg;
        const std::byte* dst_end = src_beg + sizeof(value);

        BufferCursor cursor = first_;

        while (src_pos < src_end) {
            std::span<std::byte> dst_buf = cursor.content();

            size_t cnt = std::min(src_end - src_pos, dst_buf.size_bytes());
            memcpy(dst_buf.data(), src_pos, cnt);

            size_t adv_cnt = cursor.advance(cnt);
            assert(adv_cnt == cnt);
            src_pos += cnt;
        }

        return true;
    }

}
