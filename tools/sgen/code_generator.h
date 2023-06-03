#pragma once

#include <string>
#include "ast_context.h"

struct CodeGeneratorSettings {
    std::string base_file_name;
    std::string namespace_name;
};

[[nodiscard]] std::string generate_header_file(ast::Context& context, const CodeGeneratorSettings& config);
[[nodiscard]] std::string generate_source_file(ast::Context& context, const CodeGeneratorSettings& config);
