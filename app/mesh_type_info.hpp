#ifndef OS2CX_MESH_TYPE_INFO_HPP_
#define OS2CX_MESH_TYPE_INFO_HPP_

#include <string>
#include <vector>

#include "calc.hpp"
#include "mesh_shape_info.hpp"

namespace os2cx {

enum class ElementType {
    C3D4 = 3,
};

bool valid_element_type(ElementType);
ElementType element_type_from_string(const std::string &str);

class ElementTypeInfo {
public:
    static const ElementTypeInfo &c3d4();

    static const ElementTypeInfo &get(ElementType type);

    ElementType type;
    std::string name;
    const ElementShapeInfo *shape;
};


} /* namespace os2cx */

#endif

