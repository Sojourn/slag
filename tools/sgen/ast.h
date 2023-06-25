#pragma once

#include <span>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

#define AST_NODE_ROLES(X) \
    X(STMT)               \
    X(DECL)               \
    X(TYPE)               \

#define AST_NODE_KINDS(X) \
    X(DECL,     STMT)     \
    X(COMPOUND, STMT)     \
    X(FILE,     STMT)     \
    X(VARIABLE, DECL)     \
    X(MODULE,   DECL)     \
    X(ENUM,     DECL)     \
    X(STRUCT,   DECL)     \
    X(BOOLEAN,  TYPE)     \
    X(INTEGER,  TYPE)     \
    X(STRING,   TYPE)     \
    X(BLOB,     TYPE)     \
    X(NAMED,    TYPE)     \
    X(OPTIONAL, TYPE)     \
    X(LIST,     TYPE)     \
    X(MAP,      TYPE)     \
    X(UNION,    TYPE)     \
    X(TUPLE,    TYPE)     \

namespace ast {

    class Node;

    // High-level node categories.
    enum class NodeRole : uint8_t {
#define X(AST_NODE_ROLE) \
        AST_NODE_ROLE,   \

        AST_NODE_ROLES(X)
#undef X
    };

    // Node implementations.
    enum class NodeKind : uint8_t {
#define X(AST_NODE_KIND, AST_NODE_ROLE)  \
        AST_NODE_KIND##_##AST_NODE_ROLE, \

        AST_NODE_KINDS(X)
#undef X
    };

    [[nodiscard]] const char* to_string(NodeRole role);
    [[nodiscard]] const char* to_string(NodeKind kind);
    [[nodiscard]] constexpr NodeRole to_role(NodeKind kind);

    class Node {
    protected:
        explicit Node(NodeKind kind)
            : kind_{kind}
        {
        }

    public:
        Node(Node&&) = delete;
        Node(const Node&) = delete;
        virtual ~Node() = default;

        Node& operator=(Node&&) = delete;
        Node& operator=(const Node&) = delete;

        [[nodiscard]] NodeRole role() const {
            return to_role(kind_);
        }

        [[nodiscard]] NodeKind kind() const {
            return kind_;
        }

    private:
        NodeKind kind_;
    };

    template<typename Visitor>
    void visit(Visitor& visitor, Node& node);

    template<NodeRole role>
    class NodeRoleImpl;

    using Stmt = NodeRoleImpl<NodeRole::STMT>;
    using Decl = NodeRoleImpl<NodeRole::DECL>;
    using Type = NodeRoleImpl<NodeRole::TYPE>;

    template<>
    class NodeRoleImpl<NodeRole::STMT> : public Node {
    protected:
        explicit NodeRoleImpl(NodeKind kind)
            : Node{kind}
        {
            assert(to_role(kind) == NodeRole::STMT);
        }
    };

    template<>
    class NodeRoleImpl<NodeRole::DECL> : public Node {
    protected:
        explicit NodeRoleImpl(NodeKind kind)
            : Node{kind}
        {
            assert(to_role(kind) == NodeRole::DECL);
        }

    public:
        [[nodiscard]] virtual std::string idl_type_signature() const = 0;
        [[nodiscard]] virtual std::string cpp_type_signature() const = 0;

        // TODO: think about making this another tier in the class hierarchy
        [[nodiscard]] virtual bool is_type_decl() const = 0;
    };

    template<>
    class NodeRoleImpl<NodeRole::TYPE> : public Node {
    protected:
        explicit NodeRoleImpl(NodeKind kind)
            : Node{kind}
        {
            assert(to_role(kind) == NodeRole::TYPE);
        }

    public:
        [[nodiscard]] virtual std::string idl_type_signature() const = 0;
        [[nodiscard]] virtual std::string cpp_type_signature() const = 0;
        [[nodiscard]] virtual std::vector<std::string> cpp_headers() const = 0;
    };

    template<NodeKind kind>
    class NodeKindImpl;

    using DeclStmt     = NodeKindImpl<NodeKind::DECL_STMT>;
    using CompoundStmt = NodeKindImpl<NodeKind::COMPOUND_STMT>;
    using FileStmt     = NodeKindImpl<NodeKind::FILE_STMT>;
    using VariableDecl = NodeKindImpl<NodeKind::VARIABLE_DECL>; // TODO: rename to StructMemberDecl
    using ModuleDecl   = NodeKindImpl<NodeKind::MODULE_DECL>;
    using EnumDecl     = NodeKindImpl<NodeKind::ENUM_DECL>;
    using StructDecl   = NodeKindImpl<NodeKind::STRUCT_DECL>;
    using BooleanType  = NodeKindImpl<NodeKind::BOOLEAN_TYPE>;
    using IntegerType  = NodeKindImpl<NodeKind::INTEGER_TYPE>;
    using StringType   = NodeKindImpl<NodeKind::STRING_TYPE>;
    using BlobType     = NodeKindImpl<NodeKind::BLOB_TYPE>;
    using NamedType    = NodeKindImpl<NodeKind::NAMED_TYPE>;
    using OptionalType = NodeKindImpl<NodeKind::OPTIONAL_TYPE>;
    using ListType     = NodeKindImpl<NodeKind::LIST_TYPE>;
    using MapType      = NodeKindImpl<NodeKind::MAP_TYPE>;
    using UnionType    = NodeKindImpl<NodeKind::UNION_TYPE>;
    using TupleType    = NodeKindImpl<NodeKind::TUPLE_TYPE>;

    template<>
    class NodeKindImpl<NodeKind::DECL_STMT> : public Stmt {
    public:
        NodeKindImpl(Decl& decl)
            : Stmt{NodeKind::DECL_STMT}
            , decl_{decl}
        {
        }

        [[nodiscard]] Decl& decl() {
            return decl_;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            visit(visitor, decl_);
        }

    private:
        Decl& decl_;
    };

    template<>
    class NodeKindImpl<NodeKind::COMPOUND_STMT> : public Stmt {
    public:
        NodeKindImpl(std::vector<Stmt*> stmts)
            : Stmt{NodeKind::COMPOUND_STMT}
            , stmts_{std::move(stmts)}
        {
        }

        [[nodiscard]] std::span<Stmt*> stmts() {
            return {
                stmts_.data(),
                stmts_.size(),
            };
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            for (Stmt* stmt: stmts_) {
                visit(visitor, *stmt);
            }
        }

    private:
        std::vector<Stmt*> stmts_;
    };

    template<>
    class NodeKindImpl<NodeKind::FILE_STMT> : public Stmt {
    public:
        explicit NodeKindImpl(CompoundStmt& body)
            : Stmt{NodeKind::FILE_STMT}
            , body_{body}
        {
        }

        [[nodiscard]] CompoundStmt& body() {
            return body_;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            visit(visitor, body_);
        }

    private:
        CompoundStmt& body_;
    };

    template<>
    class NodeKindImpl<NodeKind::MODULE_DECL> : public Decl {
    public:
        NodeKindImpl(std::string name, CompoundStmt& body, size_t version)
            : Decl{NodeKind::MODULE_DECL}
            , name_{std::move(name)}
            , body_{body}
            , version_{version}
        {
        }

        [[nodiscard]] const std::string& name() const {
            return name_;
        }

        [[nodiscard]] size_t version() const {
            return version_;
        }

        [[nodiscard]] CompoundStmt& body() {
            return body_;
        }

        [[nodiscard]] std::string idl_type_signature() const {
            return name_;
        }

        [[nodiscard]] std::string cpp_type_signature() const {
            return name_;
        }

        [[nodiscard]] bool is_type_decl() const {
            return false;
        }

        template<>
        struct SpoolerModule<1> {
            enum class Foo : uint8_t {
                COOL,
                STUFF,
            };
        };

        void add_decl()

        template<typename Visitor>
        void accept(Visitor& visitor) {
            visit(visitor, body_);
        }

    private:
        std::string   name_;
        CompoundStmt& body_;
        size_t        version_;
    };


    template<>
    class NodeKindImpl<NodeKind::VARIABLE_DECL> : public Decl {
    public:
        NodeKindImpl(std::string name, Type& type)
            : Decl{NodeKind::VARIABLE_DECL}
            , name_{std::move(name)}
            , type_{&type}
        {
        }

        [[nodiscard]] const std::string& name() const {
            return name_;
        }

        [[nodiscard]] Type& type() {
            return *type_;
        }

        void set_type(Type& type) {
            type_ = &type;
        }

        [[nodiscard]] std::string idl_type_signature() const override {
            return name_;
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            return name_;
        }

        [[nodiscard]] bool is_type_decl() const {
            return false;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            visit(visitor, *type_);
        }

    private:
        std::string name_;
        Type*       type_;
    };

    template<>
    class NodeKindImpl<NodeKind::ENUM_DECL> : public Decl {
    public:
        NodeKindImpl(std::string name, Type& underlying_type, std::vector<std::string> values)
            : Decl{NodeKind::ENUM_DECL}
            , name_{std::move(name)}
            , underlying_type_{&underlying_type}
            , values_{std::move(values)}
        {
        }

        [[nodiscard]] const std::string& name() const {
            return name_;
        }

        [[nodiscard]] Type& underlying_type() {
            return *underlying_type_;
        }

        void set_underlying_type(Type& underlying_type) {
            underlying_type_ = &underlying_type;
        }

        [[nodiscard]] std::span<const std::string> values() const {
            return {
                values_.data(),
                values_.size(),
            };
        }

        [[nodiscard]] std::string idl_type_signature() const override {
            return name_;
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            return name_;
        }

        [[nodiscard]] bool is_type_decl() const {
            return true;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            visit(visitor, *underlying_type_);
        }

    private:
        std::string              name_;
        Type*                    underlying_type_;
        std::vector<std::string> values_;
    };

    template<>
    class NodeKindImpl<NodeKind::STRUCT_DECL> : public Decl {
    public:
        NodeKindImpl(std::string name, CompoundStmt& body)
            : Decl{NodeKind::STRUCT_DECL}
            , name_{std::move(name)}
            , body_{body}
        {
        }

        [[nodiscard]] const std::string& name() const {
            return name_;
        }

        [[nodiscard]] CompoundStmt& body() {
            return body_;
        }

        [[nodiscard]] std::string idl_type_signature() const {
            return name_;
        }

        [[nodiscard]] std::string cpp_type_signature() const {
            return name_;
        }

        [[nodiscard]] bool is_type_decl() const {
            return true;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            visit(visitor, body_);
        }

    private:
        std::string   name_;
        CompoundStmt& body_;
    };

    template<>
    class NodeKindImpl<NodeKind::BOOLEAN_TYPE> : public Type {
    public:
        NodeKindImpl()
            : Type{NodeKind::BOOLEAN_TYPE}
        {
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            // leaf
        }

        [[nodiscard]] std::string idl_type_signature() const {
            return "boolean";
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            return "bool";
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            return {{"cstddef"}};
        }
    };

    template<>
    class NodeKindImpl<NodeKind::INTEGER_TYPE> : public Type {
    public:
        NodeKindImpl(size_t bit_count, bool is_signed)
            : Type{NodeKind::INTEGER_TYPE}
            , bit_count_{bit_count}
            , is_signed_{is_signed}
        {
        }

        [[nodiscard]] size_t bit_count() const {
            return bit_count_;
        }

        [[nodiscard]] bool is_signed() const {
            return is_signed_;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            // leaf
        }

        [[nodiscard]] std::string idl_type_signature() const {
            return fmt::format(fmt::runtime(is_signed_ ? "i{}" : "u{}"), bit_count_);
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            return fmt::format(fmt::runtime(is_signed_ ? "int{}_t" : "uint{}_t"), bit_count_);
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            return {{"cstdint"}};
        }

    private:
        size_t bit_count_;
        bool   is_signed_;
    };

    template<>
    class NodeKindImpl<NodeKind::STRING_TYPE> : public Type {
    public:
        NodeKindImpl()
            : Type{NodeKind::STRING_TYPE}
        {
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            // leaf
        }

        [[nodiscard]] std::string idl_type_signature() const {
            return "string";
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            return "std::string";
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            return {{"string"}};
        }
    };

    template<>
    class NodeKindImpl<NodeKind::BLOB_TYPE> : public Type {
    public:
        NodeKindImpl()
            : Type{NodeKind::BLOB_TYPE}
        {
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            // leaf
        }

        [[nodiscard]] std::string idl_type_signature() const {
            return "blob";
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            return "std::vector<std::byte>";
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            return {{"vector", "cstddef"}};
        }
    };

    template<>
    class NodeKindImpl<NodeKind::NAMED_TYPE> : public Type {
    public:
        explicit NodeKindImpl(std::string name)
            : Type{NodeKind::NAMED_TYPE}
            , name_{std::move(name)}
            , referenced_node_{nullptr}
        {
        }

        [[nodiscard]] const std::string& name() const {
            return name_;
        }

        // The DECL or primitive TYPE named (referenced) by this. Can be unset.
        //
        // NOTE: this is not traversed when walking the ast, as it could create a cycle.
        //
        [[nodiscard]] Node* referenced_node() {
            return referenced_node_;
        }

        void set_referenced_node(Node* referenced_node) {
            referenced_node_ = referenced_node;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            // leaf
        }

        [[nodiscard]] std::string idl_type_signature() const {
            return name_;
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            if (!referenced_node_) {
                throw std::runtime_error("NamedType is unbound");
            }

            switch (referenced_node_->role()) {
                case NodeRole::STMT: {
                    return "$"; // TODO: Implement a 'void' builtin type
                }
                case NodeRole::DECL: {
                    return static_cast<Decl*>(referenced_node_)->cpp_type_signature();
                }
                case NodeRole::TYPE: {
                    return static_cast<Type*>(referenced_node_)->cpp_type_signature();
                }
            }

            abort();
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            if (!referenced_node_) {
                throw std::runtime_error("NamedType is unbound");
            }

            switch (referenced_node_->role()) {
                case NodeRole::STMT: {
                    return {};
                }
                case NodeRole::DECL: {
                    return {};
                }
                case NodeRole::TYPE: {
                    return static_cast<Type*>(referenced_node_)->cpp_headers();
                }
            }

            abort();
        }

    private:
        std::string name_;
        Node*       referenced_node_;
    };

    template<>
    class NodeKindImpl<NodeKind::OPTIONAL_TYPE> : public Type {
    public:
        explicit NodeKindImpl(Type& value_type)
            : Type{NodeKind::OPTIONAL_TYPE}
            , value_type_{&value_type}
        {
        }

        [[nodiscard]] Type& value_type() {
            return *value_type_;
        }

        void set_value_type(Type& value_type) {
            value_type_ = &value_type;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            visit(visitor, *value_type_);
        }

        [[nodiscard]] std::string idl_type_signature() const {
            return fmt::format("optional[{}]", value_type_->idl_type_signature());
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            return fmt::format("std::optional<{}>", value_type_->cpp_type_signature());
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            return {{"optional"}};
        }

    private:
        Type* value_type_;
    };

    template<>
    class NodeKindImpl<NodeKind::LIST_TYPE> : public Type {
    public:
        explicit NodeKindImpl(Type& value_type)
            : Type{NodeKind::LIST_TYPE}
            , value_type_{&value_type}
        {
        }

        [[nodiscard]] Type& value_type() {
            return *value_type_;
        }

        void set_value_type(Type& value_type) {
            value_type_ = &value_type;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            visit(visitor, *value_type_);
        }

        [[nodiscard]] std::string idl_type_signature() const {
            return fmt::format("list[{}]", value_type_->idl_type_signature());
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            return fmt::format("std::vector<{}>", value_type_->cpp_type_signature());
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            return {{"vector"}};
        }

    private:
        Type* value_type_;
    };

    template<>
    class NodeKindImpl<NodeKind::MAP_TYPE> : public Type {
    public:
        explicit NodeKindImpl(Type& key_type, Type& mapped_type)
            : Type{NodeKind::MAP_TYPE}
            , key_type_{&key_type}
            , mapped_type_{&mapped_type}
        {
        }

        [[nodiscard]] Type& key_type() {
            return *key_type_;
        }

        void set_key_type(Type& key_type) {
            key_type_ = &key_type;
        }

        [[nodiscard]] Type& mapped_type() {
            return *mapped_type_;
        }

        void set_mapped_type(Type& mapped_type) {
            mapped_type_ = &mapped_type;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            visit(visitor, *key_type_);
            visit(visitor, *mapped_type_);
        }

        [[nodiscard]] std::string idl_type_signature() const override {
            return fmt::format(
                "map[{}, {}]",
                key_type_->idl_type_signature(),
                mapped_type_->idl_type_signature()
            );
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            return fmt::format(
                "std::unordered_map<{}, {}>",
                key_type_->cpp_type_signature(),
                mapped_type_->cpp_type_signature()
            );
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            return {{"unordered_map"}};
        }

    private:
        Type* key_type_;
        Type* mapped_type_;
    };

    template<>
    class NodeKindImpl<NodeKind::UNION_TYPE> : public Type {
    public:
        explicit NodeKindImpl(std::vector<Type*> alternative_types)
            : Type{NodeKind::UNION_TYPE}
            , alternative_types_{std::move(alternative_types)}
        {
        }

        [[nodiscard]] std::vector<Type*>& alternative_types() {
            return alternative_types_;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            for (Type* alternative_type: alternative_types_) {
                visit(visitor, *alternative_type);
            }
        }

        [[nodiscard]] std::string idl_type_signature() const override {
            std::stringstream out;

            out << "union[";
            for (size_t i = 0; i < alternative_types_.size(); ++i) {
                if (i) {
                    out << ", ";
                }

                out  << alternative_types_[i]->idl_type_signature();
            }
            out << "]";

            return out.str();
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            std::stringstream out;

            out << "std::variant<";
            out << "std::monostate"; // TODO: think more about defaulting
            for (size_t i = 0; i < alternative_types_.size(); ++i) {
                out << ", " << alternative_types_[i]->cpp_type_signature();
            }
            out << ">";

            return out.str();
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            return {{"varient"}};
        }

    private:
        std::vector<Type*> alternative_types_;
    };

    template<>
    class NodeKindImpl<NodeKind::TUPLE_TYPE> : public Type {
    public:
        explicit NodeKindImpl(std::vector<Type*> element_types)
            : Type{NodeKind::TUPLE_TYPE}
            , element_types_{std::move(element_types)}
        {
        }

        [[nodiscard]] std::vector<Type*>& element_types() {
            return element_types_;
        }

        template<typename Visitor>
        void accept(Visitor& visitor) {
            for (Type* alternative_type: element_types_) {
                visit(visitor, *alternative_type);
            }
        }

        [[nodiscard]] std::string idl_type_signature() const override {
            std::stringstream out;

            out << "tuple[";
            for (size_t i = 0; i < element_types_.size(); ++i) {
                if (i) {
                    out << ", ";
                }

                out << element_types_[i]->idl_type_signature();
            }
            out << "]";

            return out.str();
        }

        [[nodiscard]] std::string cpp_type_signature() const override {
            std::stringstream out;

            out << "std::tuple<";
            for (size_t i = 0; i < element_types_.size(); ++i) {
                if (i) {
                    out << ", ";
                }

                out << element_types_[i]->cpp_type_signature();
            }
            out << ">";

            return out.str();
        }

        [[nodiscard]] std::vector<std::string> cpp_headers() const override {
            return {{"tuple"}};
        }

    private:
        std::vector<Type*> element_types_;
    };

}

#include "ast.hpp"
