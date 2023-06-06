#include "slag/slag.h"
#include "slag/transform.h"
#include "slag/buffer_writer.h"
#include "slag/message_fragment.h"
#include "slag/message_record_fragment.h"
#include "slag/message_buffer_fragment.h"
#include <vector>
#include <span>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include "test_generated.h"

using namespace slag;

template<typename Visitor, typename T>
constexpr inline void visit(Visitor&& visitor, const T& value) {
    visitor.enter(value);
    visitor.leave(value);
}

template<typename Visitor, typename T>
constexpr inline void visit(Visitor&& visitor, const std::optional<T>& value) {
    visitor.enter(value);
    if (value) {
        visit(visitor, *value);
    }
    visitor.leave(value);
}

template<typename Visitor, typename... Types>
constexpr inline void visit(Visitor&& visitor, const std::tuple<Types...>& value) {
    visitor.enter(value);
    {
        using IndexSequence = std::make_index_sequence<sizeof...(Types)>;

        [&]<size_t... I>(std::index_sequence<I...>) {
            (visit(visitor, std::get<I>(value)), ...);
        }(IndexSequence{});
    }
    visitor.leave(value);
}

template<typename Visitor, typename... Types>
constexpr inline void visit(Visitor&& visitor, const std::variant<Types...>& value) {
    visitor.enter(value);
    std::visit([&](auto&& element) {
        visit(visitor, element);
    }, value);
    visitor.leave(value);
}

template<typename Visitor, typename T>
constexpr inline void visit(Visitor&& visitor, const std::vector<T>& value) {
    visitor.enter(value);
    for (auto&& element: value) {
        visit(visitor, element);
    }
    visitor.leave(value);
}

template<typename Visitor, typename Key, typename T>
constexpr inline void visit(Visitor&& visitor, const std::unordered_map<Key, T>& value) {
    visitor.enter(value);
    for (auto&& [key, element]: value) {
        visit(visitor, key);
        visit(visitor, element);
    }
    visitor.leave(value);
}

struct PrettyPrinter {
    void enter(bool value) {
        std::cout << "bool: " << value << std::endl;
    }
    void enter(int8_t value) {
        std::cout << "int8_t: " << (int)value << std::endl;
    }
    void enter(const std::string& value) {
        std::cout << "string: " << value << std::endl;
    }

    template<typename T>
    void enter(const T& value) {
        std::cout << "enter " << (const void*)&value << std::endl;
    }

    template<typename T>
    void leave(const T& value) {
        std::cout << "leave " << (const void*)&value << std::endl;
    }
};

template<typename Handler>
class RecordEncoder {
public:
    explicit RecordEncoder(Handler& handler)
        : handler_{handler}
    {
    }

    template<RecordType type>
    void encode(const Record<type>& record) {
        visit(*this, record);
    }

public:
    template<RecordType type>
    void enter(const Record<type>& record) {
    }

    template<RecordType type>
    void leave(const Record<type>& record) {
    }

    void enter(bool value) {
        handler_.write_value(static_cast<uint64_t>(value));
    }

    void leave(bool value) {
        (void)value;
    }

    template<typename T>
    void enter(const std::vector<T>& value) {
        handler_.write_value(value.size());
    }

    template<typename T>
    void leave(const std::vector<T>& value) {
        (void)value;
    }

private:
    Handler& handler_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    TestStruct test_struct;
    test_struct.a.push_back(0);
    test_struct.a.push_back(1);
    test_struct.a.push_back(2);
    test_struct.c = "hello";

    visit(PrettyPrinter{}, test_struct);

    return 0;
}
