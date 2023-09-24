#include "slag/core/season.h"
#include <cstdlib>
#include <cassert>

namespace slag {

    static const Season SEASONS[SEASON_COUNT] = {
        Season::SPRING,
        Season::SUMMER,
        Season::AUTUMN,
        Season::WINTER,
    };

    std::span<const Season, SEASON_COUNT> seasons() {
        return {SEASONS};
    }

    Season next(Season season) {
        return static_cast<Season>(
            (to_index(season) + 1) % SEASON_COUNT
        );
    }

    Season prev(Season season) {
        return static_cast<Season>(
            (to_index(season) - 1) % SEASON_COUNT
        );
    }

    std::string_view to_string(Season season) {
        using namespace std::string_view_literals;

        switch (season) {
            case Season::SPRING: return "SPRING"sv;
            case Season::SUMMER: return "SUMMER"sv;
            case Season::AUTUMN: return "AUTUMN"sv;
            case Season::WINTER: return "WINTER"sv;
        }

        abort();
    }

    size_t to_index(Season season) {
        return static_cast<size_t>(season);
    }

}
