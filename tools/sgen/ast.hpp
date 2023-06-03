#include <cassert>

namespace ast {

    inline const char* to_string(NodeRole role) {
        switch (role) {
#define X(AST_NODE_ROLE)                    \
            case NodeRole::AST_NODE_ROLE: { \
                return #AST_NODE_ROLE;      \
            }                               \

            AST_NODE_ROLES(X)
#undef X
        }

        abort();
    }

    inline const char* to_string(NodeKind kind) {
        switch (kind) {
#define X(AST_NODE_KIND, AST_NODE_ROLE)                       \
            case NodeKind::AST_NODE_KIND##_##AST_NODE_ROLE: { \
                return #AST_NODE_KIND "_" #AST_NODE_ROLE;     \
            }                                                 \

            AST_NODE_KINDS(X)
#undef X
        }

        abort();
    }

    inline constexpr NodeRole to_role(NodeKind kind) {
        switch (kind) {
#define X(AST_NODE_KIND, AST_NODE_ROLE)                       \
            case NodeKind::AST_NODE_KIND##_##AST_NODE_ROLE: { \
                return NodeRole::AST_NODE_ROLE;               \
            }                                                 \

            AST_NODE_KINDS(X)
#undef X
        }

        abort();
    }

    template<typename Visitor>
    inline void visit(Visitor& visitor, Node& node) {
        switch (node.kind()) {
#define X(AST_NODE_KIND, AST_NODE_ROLE)                                               \
            case NodeKind::AST_NODE_KIND##_##AST_NODE_ROLE: {                         \
                using Impl = NodeKindImpl<NodeKind::AST_NODE_KIND##_##AST_NODE_ROLE>; \
                Impl& impl = static_cast<Impl&>(node);                                \
                visitor.enter(impl);                                                  \
                impl.accept(visitor);                                                 \
                visitor.leave(impl);                                                  \
                break;                                                                \
            }                                                                         \

            AST_NODE_KINDS(X)
#undef X
        }
    }


}
