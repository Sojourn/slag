#include "test_generated.h"

namespace slag {

    std::string_view to_string(TestEnum value) {
        using namespace std::string_view_literals;

        switch (value) {
            case TestEnum::FOO: return "FOO"sv;
            case TestEnum::BAR: return "BAR"sv;
            case TestEnum::BAZ: return "BAZ"sv;
        }

        return "UNKNOWN"sv;
    }

}

