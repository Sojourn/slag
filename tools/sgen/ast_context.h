#pragma once

#include <memory>
#include <vector>
#include <string>
#include <utility>
#include "ast.h"

namespace ast {

    class Context {
        Context(Context&&) = delete;
        Context(const Context&) = delete;
        Context& operator=(Context&&) = delete;
        Context& operator=(const Context&) = delete;

    public:
        Context() {
            init_builtin_types();
        }

        std::span<Node*> nodes() {
            return nodes_;
        }

        template<NodeRole role>
        [[nodiscard]] std::span<NodeRoleImpl<role>*> nodes() {
            auto&& index = std::get<static_cast<size_t>(role)>(node_role_index_);
            return {
                index.data(),
                index.size(),
            };
        }

        template<NodeKind kind>
        [[nodiscard]] std::span<NodeKindImpl<kind>*> nodes() {
            auto&& index = std::get<static_cast<size_t>(kind)>(node_kind_index_);
            return {
                index.data(),
                index.size(),
            };
        }

        [[nodiscard]] std::span<Type*> builtin_types() {
            return {
                builtin_types_.data(),
                builtin_types_.size(),
            };
        }

    public:
        [[nodiscard]] DeclStmt& new_decl_stmt(Decl& decl) {
            return make_node<NodeKind::DECL_STMT>(decl);
        }

        [[nodiscard]] CompoundStmt& new_compound_stmt(std::vector<Stmt*> stmts) {
            return make_node<NodeKind::COMPOUND_STMT>(std::move(stmts));
        }

        [[nodiscard]] FileStmt& new_file_stmt(CompoundStmt& body) {
            return make_node<NodeKind::FILE_STMT>(body);
        }

        [[nodiscard]] VariableDecl& new_variable_decl(std::string name, Type& type) {
            return make_node<NodeKind::VARIABLE_DECL>(std::move(name), type);
        }

        [[nodiscard]] EnumDecl& new_enum_decl(std::string name, std::string underlying_type_name, std::vector<std::string> values) {
            return new_enum_decl(
                std::move(name),
                new_named_type(std::move(underlying_type_name)),
                std::move(values)
            );
        }

        [[nodiscard]] EnumDecl& new_enum_decl(std::string name, Type& underlying_type, std::vector<std::string> values) {
            return make_node<NodeKind::ENUM_DECL>(std::move(name), underlying_type, std::move(values));
        }

        [[nodiscard]] StructDecl& new_struct_decl(std::string name, CompoundStmt& body) {
            return make_node<NodeKind::STRUCT_DECL>(std::move(name), body);
        }

        [[nodiscard]] BooleanType& new_boolean_type() {
            return make_node<NodeKind::BOOLEAN_TYPE>();
        }

        [[nodiscard]] IntegerType& new_integer_type(size_t bit_count, bool is_signed) {
            return make_node<NodeKind::INTEGER_TYPE>(bit_count, is_signed);
        }

        [[nodiscard]] StringType& new_string_type() {
            return make_node<NodeKind::STRING_TYPE>();
        }

        [[nodiscard]] BlobType& new_blob_type() {
            return make_node<NodeKind::BLOB_TYPE>();
        }

        [[nodiscard]] NamedType& new_named_type(std::string name) {
            return make_node<NodeKind::NAMED_TYPE>(name);
        }

        [[nodiscard]] OptionalType& new_optional_type(Type& value_type) {
            return make_node<NodeKind::OPTIONAL_TYPE>(value_type);
        }

        [[nodiscard]] MapType& new_map_type(Type& key_type, Type& mapped_type) {
            return make_node<NodeKind::MAP_TYPE>(key_type, mapped_type);
        }

        [[nodiscard]] ListType& new_list_type(Type& value_type) {
            return make_node<NodeKind::LIST_TYPE>(value_type);
        }

        [[nodiscard]] UnionType& new_union_type(std::vector<Type*> alternative_types) {
            return make_node<NodeKind::UNION_TYPE>(std::move(alternative_types));
        }

        [[nodiscard]] TupleType& new_tuple_type(std::vector<Type*> element_types) {
            return make_node<NodeKind::TUPLE_TYPE>(std::move(element_types));
        }

    private:
        void init_builtin_types() {
            builtin_types_.push_back(&new_string_type());
            builtin_types_.push_back(&new_blob_type());
            builtin_types_.push_back(&new_boolean_type());
            for (size_t bit_count: {8, 16, 32, 64}) {
                for (bool is_signed: {false, true}) {
                    builtin_types_.push_back(&new_integer_type(bit_count, is_signed));
                }
            }
        }

        template<NodeKind kind, typename... Args>
        [[nodiscard]] NodeKindImpl<kind>& make_node(Args&&... args) {
            static constexpr size_t ROLE_POSITION = static_cast<size_t>(to_role(kind));
            static constexpr size_t KIND_POSITION = static_cast<size_t>(kind);

            auto&& ptr = std::make_unique<NodeKindImpl<kind>>(std::forward<Args>(args)...);
            auto&& ref = *ptr;

            // index the node
            nodes_.push_back(&ref);
            std::get<ROLE_POSITION>(node_role_index_).push_back(&ref);
            std::get<KIND_POSITION>(node_kind_index_).push_back(&ref);

            storage_.push_back(std::move(ptr));
            return ref;
        }

    private:
        using NodeRoleIndex = std::tuple<
#define X(AST_NODE_ROLE)                                         \
            std::vector<NodeRoleImpl<NodeRole::AST_NODE_ROLE>*>, \

            AST_NODE_ROLES(X)
#undef X
            int
        >;

        using NodeKindIndex = std::tuple<
#define X(AST_NODE_KIND, AST_NODE_ROLE)                                            \
            std::vector<NodeKindImpl<NodeKind::AST_NODE_KIND##_##AST_NODE_ROLE>*>, \

            AST_NODE_KINDS(X)
#undef X
            int
        >;

        std::vector<Node*>                 nodes_;
        std::vector<Type*>                 builtin_types_;
        NodeRoleIndex                      node_role_index_;
        NodeKindIndex                      node_kind_index_;
        std::vector<std::unique_ptr<Node>> storage_; // TODO: use a bump-pointer allocator
    };

}
