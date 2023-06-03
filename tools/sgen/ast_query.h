#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "ast.h"
#include "ast_context.h"

namespace ast {

    [[nodiscard]] std::unordered_set<std::string> query_distinct_idl_type_signatures(Context& context);
    [[nodiscard]] std::unordered_multimap<std::string, Type*> query_idl_type_signatures(Context& context);

    // TODO: move this into Context
    template<NodeKind... kinds>
    [[nodiscard]] std::vector<Node*> query_nodes(Context& context);
    [[nodiscard]] std::vector<Decl*> query_type_decls(Context& context);
    [[nodiscard]] std::unordered_set<std::string> query_cpp_headers(Context& context);

    [[nodiscard]] std::vector<VariableDecl*> query_struct_fields(StructDecl& struct_decl);

}

#include "ast_query.hpp"
