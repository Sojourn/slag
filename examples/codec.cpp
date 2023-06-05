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

class RecordPrettyPrinter {
public:
    RecordPrettyPrinter(std::string_view name, const Record<type>& record) {
        this->operator()(name, record);
    }

    [[nodiscard]] std::string output() const {
        return out_.str();
    }

public:
    template<RecordType type>
    void operator()(std::string_view name, const Record<type>& value) {
        line("{}: Record<{}> {{", name, to_string(type));
        {
            indent();
            visit(*this, value);
            dedent();
        }
        line("}};");
    }

    template<typename... Ts>
    void operator()(std::string_view name, const std::tuple<Ts...>& value) {
        line("{}: tuple", name);
    }

    template<typename... Ts>
    void operator()(std::string_view name, const std::variant<Ts...>& value) {
        line("{}: variant", name);
    }

    template<typename T>
    void operator()(std::string_view name, const std::vector<T>& value) {
        line("{}: [", name);
        {
            indent();
            for (size_t i = 0; i < values.size(); ++i) {
                std::string index_name = fmt::format("{}", i);
                this->operator()(index_name, values[i]);
            }
            dedent();
        }
        line("];");
    }

    template<typename K, typename V>
    void operator()(std::string_view name, const std::unordered_map<K, V>& value) {
        line("{}: {", name);
        {
            indent();
            for (size_t i = 0; i < values.size(); ++i) {
                this->operator()(index_name, values[i]);
            }
            dedent();
        }
        line("};");
    }

    template<typename T>
    void operator()(std::string_view name, const T& value) {
        line("{}: {}", name, value);
    }

private:
    void indent() {
        ++indent_level_;
    }

    void dedent() {
        --indent_level_;
    }

    template<typename... Args>
    void line(const char* fmt, Args&&... args) {
        for (size_t i = 0; i < indent_level_; ++i) {
            out_ << "    ";
        }

        out_ << fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...) << '\n';
    }

    void line() {
        out_ << '\n';
    }

private:
    std::stringstream out_;
    size_t            indent_level_ = 0;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    TestStruct test_struct;
    test_struct.c = "hello";

    visit_fields(test_struct, []<typename T>(std::string_view name, const T& value) {
        if constexpr (std::is_same_v<T, std::string>) {
            std::cout << name << "=" << value << std::endl;
        }
    });

    return 0;
}
