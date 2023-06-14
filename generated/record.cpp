#include "record.h"

namespace slag {

    std::optional<std::string_view> to_string(TestEnum value) {
        using namespace std::string_view_literals;

        switch (value) {
            case TestEnum::FOO: return "FOO"sv;
            case TestEnum::BAR: return "BAR"sv;
            case TestEnum::BAZ: return "BAZ"sv;
        }

        return std::nullopt;
    }

    std::optional<std::string_view> to_string(RecordType value) {
        using namespace std::string_view_literals;

        switch (value) {
            case RecordType::HEADER: return "HEADER"sv;
            case RecordType::TEST_STRUCT: return "TEST_STRUCT"sv;
        }

        return std::nullopt;
    }

}

