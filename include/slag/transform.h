#pragma once

#include <cstdint>
#include <cstddef>

namespace slag {

    constexpr inline uint64_t encode_zig_zag(int64_t decoded_value) {
        return (static_cast<uint64_t>(decoded_value) << 1) ^ -(static_cast<uint64_t>(decoded_value) >> 63);
    }

    constexpr inline int64_t decode_zig_zag(uint64_t encoded_value) {
        return static_cast<int64_t>((encoded_value >> 1) ^ -(encoded_value & 1));
    }

    constexpr inline uint64_t encode_delta(uint64_t decoded_value, uint64_t last_encoded_value) {
        return decoded_value - last_encoded_value;
    }

    constexpr inline uint64_t decode_delta(uint64_t encoded_value, uint64_t last_decoded_value) {
        return encoded_value + last_decoded_value;
    }

}
