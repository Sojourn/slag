#include "catch.hpp"
#include "slag/slag.h"

using namespace slag;

TEST_CASE("result lifecycle", "[Result]") {
    int value = 13;
    Result<int> value_result{value};
    CHECK(value_result.value() == value);

    Error error{ErrorCode::CANCELED};
    Result<int> error_result{error};
    CHECK(error_result.error() == error);

    SECTION("copy construction") {
        Result<int> value_result_copy{value_result};
        CHECK(value_result_copy.value() == value);

        Result<int> error_result_copy{error_result};
        CHECK(error_result_copy.error() == error);
    }

    SECTION("move construction") {
        Result<int> value_result_copy{std::move(value_result)};
        CHECK(value_result_copy.value() == value);

        Result<int> error_result_copy{std::move(error_result)};
        CHECK(error_result_copy.error() == error);
    }

    SECTION("copy assignment") {
        Result<int> value_result_copy{0};
        value_result_copy = value_result;
        CHECK(value_result_copy.value() == value);

        Result<int> error_result_copy{Error{}};
        error_result_copy = error_result;
        CHECK(error_result_copy.error() == error);
    }

    SECTION("move assignment") {
        Result<int> value_result_copy{0};
        value_result_copy = std::move(value_result);
        CHECK(value_result_copy.value() == value);

        Result<int> error_result_copy{Error{}};
        error_result_copy = std::move(error_result);
        CHECK(error_result_copy.error() == error);
    }
}
