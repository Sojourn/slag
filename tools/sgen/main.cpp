#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include "ast.h"
#include "ast_context.h"
#include "ast_query.h"
#include "ast_transform.h"
#include "code_generator.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern int yyparse(ast::Context& context);

void yyerror(ast::Context& context, const char* text) {
    std::cerr << "yyerror " << text << std::endl;
    exit(EXIT_FAILURE);
}

class PrettyPrint {
public:
    void enter(ast::StructDecl& node) {
        indent(); std::cout << to_string(node.kind()) << " name:" << node.name() << std::endl;
        ++depth_;
    }

    void enter(ast::EnumDecl& node) {
        indent(); std::cout << to_string(node.kind()) << " name:" << node.name() << std::endl;
        ++depth_;
    }

    void enter(ast::VariableDecl& node) {
        indent(); std::cout << to_string(node.kind()) << " name:" << node.name() << std::endl;
        ++depth_;
    }

    void enter(ast::NamedType& node) {
        indent(); std::cout << to_string(node.kind()) << " name:" << node.name() << std::endl;
        ++depth_;
    }

    void enter(ast::IntegerType& node) {
        indent(); std::cout << to_string(node.kind()) << " bit_count:" << node.bit_count() << " is_signed:" << node.is_signed() << std::endl;
        ++depth_;
    }

    void enter(ast::Node& node) {
        indent(); std::cout << to_string(node.kind()) << std::endl;
        ++depth_;
    }

    void leave(ast::Node& node) {
        --depth_;
    }

private:
    void indent() {
        for (size_t i = 0; i < (depth_ * 2); ++i) {
            std::cout << ' ';
        }
    }

private:
    size_t depth_ = 0;
};

void write_file(const std::string& file_path, std::span<const std::byte> buffer) {
    int file_descriptor = ::open(file_path.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0644);
    if (file_descriptor < 0) {
        throw std::runtime_error(fmt::format("Failed to open file: {}", strerror(errno)));
    }

    ssize_t bytes_written = ::write(file_descriptor, buffer.data(), buffer.size_bytes());
    if (bytes_written < 0) {
        throw std::runtime_error(fmt::format("Failed to write file: {}", strerror(errno)));
    }
    if (bytes_written < buffer.size_bytes()) {
        throw std::runtime_error("Failed to write file: truncated write");
    }

    int rc = ::close(file_descriptor);
    assert(rc >= 0);
}

void write_file(const std::string& file_path, const std::string& text) {
    auto buffer = std::as_bytes(
        std::span(
            text.data(),
            text.size()
        )
    );

    write_file(file_path, buffer);
}

int main(int argc, char** argv) {
    ast::Context context;

    try {
        int result = yyparse(context);
        (void)result; // TODO: figure out how to interpret this
    }
    catch (const std::exception& ex) {
        std::cerr << "Parsing failed: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    try {
        ast::resolve_named_types(context);
    }
    catch (const std::exception& ex) {
        std::cerr << "Type resolution failed: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    try {
        CodeGeneratorSettings settings {
            .base_file_name = "test_generated",
            .namespace_name = "slag",
        };

        std::string output_directory = "../../examples";

        write_file(
            fmt::format("{}/{}.h", output_directory, settings.base_file_name),
            generate_header_file(context, settings)
        );

        write_file(
            fmt::format("{}/{}.cpp", output_directory, settings.base_file_name),
            generate_source_file(context, settings)
        );
    }
    catch (const std::exception& ex) {
        std::cerr << "Code generation failed: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
