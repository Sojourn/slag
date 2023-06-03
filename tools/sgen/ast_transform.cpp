#include "ast_transform.h"
#include "ast.h"
#include "ast_query.h"
#include "ast_context.h"
#include "string_util.h"
#include <string>
#include <unordered_map>
#include <cassert>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

namespace ast {

    void create_record_type_enum(Context& context) {
        std::vector<std::string> values;
        for (StructDecl* struct_decl: context.nodes<NodeKind::STRUCT_DECL>()) {
            values.push_back(
                to_record_type(struct_decl->name())
            );
        }

        (void)context.new_enum_decl("RecordType", "u16", std::move(values));
    }

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
