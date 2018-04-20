#include "mesh_type_info.hpp"

#include <assert.h>

#include <stdexcept>

namespace os2cx {

bool valid_element_type(ElementType type) {
    switch (type) {
    case ElementType::C3D4:
        return true;
    default:
        return false;
    }
}

ElementType element_type_from_string(const std::string &str) {
    if (str == "C3D4") return ElementType::C3D4;
    throw std::domain_error("no such element type: " + str);
}

const ElementTypeInfo &ElementTypeInfo::c3d4() {
    static ElementTypeInfo info {
        ElementType::C3D4,
        "C3D4",
        &ElementShapeInfo::tetrahedron4()
    };
    return info;
}

const ElementTypeInfo &ElementTypeInfo::get(ElementType type) {
    switch (type) {
    case ElementType::C3D4: return c3d4();
    default: assert(false);
    }
}

} /* namespace os2cx */

