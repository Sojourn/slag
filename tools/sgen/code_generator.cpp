#include "code_generator.h"
#include "string_util.h"
#include "ast.h"
#include "ast_query.h"
#include "ast_context.h"
#include <cstring>
#include <cassert>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

using namespace ast;

class CodeGenerator {
public:
    [[nodiscard]] std::string output() const {
        return out_.str();
    }

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

class Level {
    Level(Level&&) = delete;
    Level(const Level&) = delete;
    Level& operator=(Level&&) = delete;
    Level& operator=(const Level&) = delete;

public:
    Level(CodeGenerator& generator)
        : generator_{generator}
    {
        generator_.indent();
    }

    ~Level() {
        generator_.dedent();
    }

private:
    CodeGenerator& generator_;
};

class HeaderCodeGenerator : public CodeGenerator {
public:
    HeaderCodeGenerator(Context& context, const CodeGeneratorSettings& settings)
        : context_{context}
        , settings_{settings}
    {
        line("#pragma once");
        line();
        line("#include <string>");
        line("#include <string_view>");
        line("#include <tuple>");
        line("#include <vector>");
        line("#include <variant>");
        line("#include <optional>");
        line("#include <unordered_set>");
        line("#include <unordered_map>");
        line("#include <cstddef>");
        line("#include <cstdint>");
        line();

        line("namespace {} {{", settings_.namespace_name);
        {
            Level namespace_level{*this};
            line();

            generate_enum_declarations();
            generate_enum_to_string_function_prototypes();
            generate_forward_declarations();
            generate_struct_declarations();
            generate_struct_field_visitors();

            generate_record_info();
            generate_record_field_info();
        }
        line("}}");
        line();
    }

private:
    void generate_forward_declarations() {
        line("template<RecordType type>");
        line("struct Record;");
        line();

        line("template<RecordType type>");
        line("struct RecordInfo;");
        line();

        line("template<RecordType type, size_t index>");
        line("struct RecordFieldInfo;");
        line();

        for (StructDecl* struct_decl: context_.nodes<NodeKind::STRUCT_DECL>()) {
            line(
                "using {} = Record<RecordType::{}>;",
                struct_decl->name(),
                to_record_type(struct_decl->name())
            );
        }

        line();
    }

    void generate_enum_declarations() {
        for (EnumDecl* enum_decl: context_.nodes<NodeKind::ENUM_DECL>()) {
            Type& underlying_type = enum_decl->underlying_type();

            line("enum class {} : {} {{", enum_decl->name(), underlying_type.cpp_type_signature());
            {
                Level enum_level{*this};

                for (const std::string& value: enum_decl->values()) {
                    line("{},", value);
                }
            }
            line("}};");
            line();
        }
    }

    void generate_enum_to_string_function_prototypes() {
        for (EnumDecl* enum_decl: context_.nodes<NodeKind::ENUM_DECL>()) {
            line("[[nodiscard]] std::optional<std::string_view> to_string({} value);", enum_decl->name());
        }

        line();
    }

    void generate_struct_declarations() {
        for (StructDecl* struct_decl: context_.nodes<NodeKind::STRUCT_DECL>()) {
            line("template<>");
            line("struct Record<RecordType::{}> {{", to_record_type(struct_decl->name()));
            {
                Level struct_level{*this};

                for (VariableDecl* field: query_struct_fields(*struct_decl)) {
                    Type& field_type = field->type();
                    line("{} {} = {{}};", field_type.cpp_type_signature(), field->name());
                }
            }
            line("}};");
            line();
        }
    }

    void generate_struct_field_visitors() {
        for (const char* const_modifier: {"", "const "}) {
            for (StructDecl* struct_decl: context_.nodes<NodeKind::STRUCT_DECL>()) {
                line("template<typename Visitor>");
                line("constexpr inline void visit_fields({}{}& object, Visitor&& visitor) {{", const_modifier, struct_decl->name());
                {
                    Level function_level{*this};

                    line("using namespace std::string_view_literals;");
                    line();
                    for (VariableDecl* field: query_struct_fields(*struct_decl)) {
                        line("visitor(\"{}\"sv, object.{});", field->name(), field->name());
                    }
                }
                line("}}");
                line();
            }
        }
    }

    void generate_record_info() {
        for (StructDecl* struct_decl: context_.nodes<NodeKind::STRUCT_DECL>()) {
            auto record_type = to_record_type(struct_decl->name());
            auto record_fields = query_struct_fields(*struct_decl);

            line("template<>");
            line("struct RecordInfo<RecordType::{}> {{", record_type);
            {
                Level function_level{*this};

                line("constexpr static std::string_view type_name = to_string(RecordType::{});", record_type);
                line("constexpr static size_t field_count = {};", record_fields.size());
            }
            line("}}");
            line();

            for (size_t record_field_index = 0; record_field_index < record_fields.size(); ++record_field_index) {
                VariableDecl* variable_decl = record_fields[record_field_index];

                line("template<>");
                line("struct RecordFieldInfo<RecordType::{}, {}> {{", record_type, record_field_index);
                {
                    Level function_level{*this};

                    line(
                        "constexpr static std::string_view name = \"{}\";",
                        variable_decl->name()
                    );
                    line(
                        "constexpr static {} Record<RecordType::{}>::*accessor = &Record<RecordType::{}>::{};",
                        variable_decl->type().cpp_type_signature(),
                        record_type,
                        record_type,
                        variable_decl->name()
                    );
                }
                line("}}");
                line();
            }
        }
    }

    void generate_record_field_info() {
    }

private:
    Context&              context_;
    CodeGeneratorSettings settings_;
};

class SourceCodeGenerator : public CodeGenerator {
public:
    SourceCodeGenerator(Context& context, const CodeGeneratorSettings& settings)
        : context_{context}
        , settings_{settings}
    {
        line("#include \"{}.h\"", settings_.base_file_name);
        line();

        line("namespace {} {{", settings_.namespace_name);
        {
            Level namespace_level{*this};
            line();

            generate_enum_to_string_functions();
        }
        line("}}");
        line();
    }

private:
    void generate_enum_to_string_functions() {
        for (EnumDecl* enum_decl: context_.nodes<NodeKind::ENUM_DECL>()) {
            line("std::optional<std::string_view> to_string({} value) {{", enum_decl->name());
            {
                Level function_level{*this};

                line("using namespace std::string_view_literals;");
                line();
                line("switch (value) {{");
                {
                    Level switch_level{*this};

                    for (const std::string& value: enum_decl->values()) {
                        line("case {}::{}: return \"{}\"sv;", enum_decl->name(), value, value);
                    }
                }
                line("}}");
                line();
                line("return std::nullopt;");
            }
            line("}}");
            line();
        }
    }

private:
    Context&              context_;
    CodeGeneratorSettings settings_;
};

std::string generate_header_file(Context& context, const CodeGeneratorSettings& settings) {
    return HeaderCodeGenerator{context, settings}.output();
}

std::string generate_source_file(Context& context, const CodeGeneratorSettings& settings) {
    return SourceCodeGenerator{context, settings}.output();
}
