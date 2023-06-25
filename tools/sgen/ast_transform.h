#pragma once

// TODO: think about naming this ast_operations

namespace ast {

    class Context;

    void create_module_type_enum(Context& context);
    void create_record_type_enum(Context& context);

    // Determines what each NamedType node is referencing, and binds it.
    void resolve_named_types(Context& context);

}
