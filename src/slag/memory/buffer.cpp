#include "buffer.h"
#include <cassert>

namespace slag {

#if 0
    bool advance_cursor(BufferCursor& cursor, size_t count) {
        const Buffer& buffer = *cursor.buffer;
        if (buffer.size < (cursor.buffer_offset + count)) {
            return false;
        }

        for (size_t total_remainder = count; total_remainder > 0; ) {
            const BufferSegment& segment = *cursor.segment;

            // Calculate how much of the remaining count is in the current segment.
            size_t segment_remainder = std::min(
                segment.content.size_bytes() - cursor.segment_offset,
                total_remainder
            );

            bool skip_segment = segment_remainder < total_remainder;
            total_remainder -= segment_remainder;

            // Advance over or within the current segment.
            if (skip_segment) {
                cursor.segment = segment.next;
                cursor.segment_offset = 0;
            }
            else {
                cursor.segment_offset += segment_remainder;
                break;
            }
        }

        // Calculate this directly instead of inductively.
        cursor.buffer_offset += count;

        return true;
    }
#endif

}
