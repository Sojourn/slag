#include "ast.h"
#include "ast_query.h"
#include "ast_context.h"
#include "ast_transform.h"
#include <string>
#include <unordered_map>
#include <cassert>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

namespace ast {

    void resolve_named_types(Context& context) {
        std::unordered_map<std::string, ast::Node*> targets;

        // register builtin types
        for (Type* type: context.builtin_types()) {
            auto&& [_, inserted] = targets.insert(std::make_pair(type->idl_type_signature(), type));
            assert(inserted);
        }

        // register declared types
        for (Decl* decl: query_type_decls(context)) {
            auto&& [it, inserted] = targets.insert(std::make_pair(decl->idl_type_signature(), decl));
            if (!inserted) {
                throw std::runtime_error(
                    fmt::format(
                        "Conflicting declarations {}/{} and {}/{}"
                        , to_string(it->second->kind()), it->first
                        , to_string(decl->kind()), it->first
                    )
                );
            }
        }

        // iterate over nodes naming a type and attempt to resolve them
        for (NamedType* named_type: context.nodes<NodeKind::NAMED_TYPE>()) {
            auto&& idl_type_signature = named_type->idl_type_signature();

            if (auto it = targets.find(idl_type_signature); it != targets.end()) {
                // resolved; bind the node being referenced
                named_type->set_referenced_node(it->second);
            }
            else {
                // failed to resolve
                throw std::runtime_error(
                    fmt::format("Unknown type '{}'", idl_type_signature)
                );
            }
        }
    }

}
