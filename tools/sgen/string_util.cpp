#include "string_util.h"
#include <sstream>
#include <cstdlib>
#include <clocale>
#include <cctype>

std::string to_upper(std::string_view text) {
    std::stringstream out;
    for (char c: text) {
        out << static_cast<char>(std::toupper(static_cast<int>(c)));
    }

    return out.str();
}

std::string to_lower(std::string_view text) {
    std::stringstream out;
    for (char c: text) {
        out << static_cast<char>(std::tolower(static_cast<int>(c)));
    }

    return out.str();
}

std::string pascal_to_snake_case(std::string_view text) {
    std::stringstream out;

    enum class State {
        SCANNING_FOR_LOWER,
        SCANNING_FOR_UPPER,
    } state = State::SCANNING_FOR_LOWER;

    for (char c: text) {
        switch (state) {
            case State::SCANNING_FOR_LOWER: {
                if (std::islower(c)) {
                    state = State::SCANNING_FOR_UPPER;
                }
                break;
            }
            case State::SCANNING_FOR_UPPER: {
                if (std::isupper(c)) {
                    state = State::SCANNING_FOR_LOWER;
                    out << '_';
                }
                break;
            }
        }

        out << static_cast<char>(std::tolower(c));
    }

    return out.str();
}

std::string to_record_type(std::string_view record_name) {
    return to_upper(
        pascal_to_snake_case(record_name)
    );
}
