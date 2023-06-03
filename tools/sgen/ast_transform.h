#pragma once

// TODO: think about naming this ast_operations

namespace ast {

    class Context;

    // Determines what each NamedType node is referencing, and binds it.
    void resolve_named_types(Context& context);
    
}
