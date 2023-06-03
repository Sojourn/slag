#include "type.h"
#include <sstream>

const char* to_string(TypeKind kind) {
    switch (kind) {
#define X(TYPE_KIND) case TypeKind::TYPE_KIND: return #TYPE_KIND;
        TYPE_KINDS(X)
#undef X
    }

    abort(); // unreachable
}

const char* to_string(PrimitiveType primitive_type) {
    switch (primitive_type) {
#define X(PRIMITIVE_TYPE) case PrimitiveType::PRIMITIVE_TYPE: return #PRIMITIVE_TYPE;
        PRIMITIVE_TYPES(X)
#undef X
    }

    abort(); // unreachable
}

TypeSystem::TypeSystem() {
#define X(PRIMITIVE_TYPE) \
    initialize_primitive_type<PrimitiveType::PRIMITIVE_TYPE>();

    PRIMITIVE_TYPES(X)
#undef X
}

void TypeSystem::bind_type(const std::string& type_name, const Type& type) {
    auto&& [_, inserted] = type_index_.insert(
        std::make_pair(type_name, &type)
    );
    if (!inserted) {
        throw std::runtime_error("Duplicate type");
    }
}

const Type* TypeSystem::find_primitive_type(PrimitiveType primitive_type) const {
    return type_index_.at(to_string(primitive_type));
}

const Type* TypeSystem::find_type(const std::string& type_name) const {
    if (auto it = type_index_.find(type_name); it != type_index_.end()) {
        return it->second;
    }

    return nullptr;
}

template<PrimitiveType primitive_type>
void TypeSystem::initialize_primitive_type() {
    bind_type(
        to_string(primitive_type),
        create_type<TypeKind::PRIMITIVE>(primitive_type)
    );
}

std::string to_parameter_string(const Types& types) {
    std::stringstream stream;
    stream << '[';
    for (size_t i = 0; i < types.size(); ++i) {
        if (i > 0) {
            stream << ',';
        }

        stream << to_string(types[i]->kind());
    }
    stream << ']';

    return stream.str();
}
