#pragma once

#include <span>
#include <string_view>
#include <cstdint>
#include <cstddef>

namespace slag::postal {

    constexpr size_t SEASON_COUNT = 4;

    enum class Season : uint8_t {
        SPRING,
        SUMMER,
        AUTUMN,
        WINTER,
    };

    std::span<const Season, SEASON_COUNT> seasons();

    Season next(Season season);
    Season prev(Season season);

    std::string_view to_string(Season season);
    size_t to_index(Season season);

}
