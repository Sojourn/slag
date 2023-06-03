#pragma once

#include <string_view>
#include <string>

[[nodiscard]] std::string to_upper(std::string_view text);
[[nodiscard]] std::string to_lower(std::string_view text);
[[nodiscard]] std::string pascal_to_snake_case(std::string_view text);

[[nodiscard]] std::string to_record_type(std::string_view record_name);
