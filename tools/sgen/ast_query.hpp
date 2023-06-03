#include <algorithm>
#include <cassert>

namespace ast {

    inline std::unordered_set<std::string> query_distinct_idl_type_signatures(Context& context) {
        std::unordered_set<std::string> result;
        for (Type* type: context.nodes<NodeRole::TYPE>()) {
            result.emplace(type->idl_type_signature());
        }

        return result;
    }

    inline std::unordered_multimap<std::string, Type*> query_idl_type_signatures(Context& context) {
        std::unordered_multimap<std::string, Type*> result;
        for (Type* type: context.nodes<NodeRole::TYPE>()) {
            result.emplace(type->idl_type_signature(), type);
        }

        return result;
    }

    template<NodeKind... kinds>
    inline std::vector<Node*> query_nodes(Context& context) {
        std::vector<Node*> result;
        for (Node* node: context.nodes()) {
            for (NodeKind kind: {kinds...}) {
                if (node->kind() ==  kind) {
                    result.push_back(node);
                }
            }
        }

        return result;
    }

    inline std::vector<Decl*> query_type_decls(Context& context) {
        std::vector<Decl*> result;
        for (Decl* decl: context.nodes<NodeRole::DECL>()) {
            if (decl->is_type_decl()) {
                result.push_back(decl);
            }
        }

        return result;
    }

    inline std::unordered_set<std::string> query_cpp_headers(Context& context) {
        std::unordered_set<std::string> result;
        for (Type* type: context.nodes<NodeRole::TYPE>()) {
            for (const std::string& cpp_header: type->cpp_headers()) {
                result.insert(cpp_header);
            }
        }

        return result;
    }

    inline std::vector<VariableDecl*> query_struct_fields(StructDecl& struct_decl) {
        struct {
            std::vector<VariableDecl*> result;

            void enter(VariableDecl& variable_decl) {
                result.push_back(&variable_decl);
            }

            void enter(Node&) {}
            void leave(Node&) {}
        } query;

        visit(query, struct_decl);

        return std::move(query.result);
    }

}
