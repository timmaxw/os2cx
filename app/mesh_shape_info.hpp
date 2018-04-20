#ifndef OS2CX_MESH_SHAPE_INFO_HPP_
#define OS2CX_MESH_SHAPE_INFO_HPP_

#include <vector>

#include "calc.hpp"

namespace os2cx {

class ElementShapeInfo {
public:
    enum class Node { Corner, Edge };

    static const int max_faces_per_element = 6;
    static const int max_nodes_per_face = 8;

    static const ElementShapeInfo &tetrahedron4();

    std::vector<Node> nodes;
    std::vector<std::vector<int> > faces;

    virtual Volume volume(const Point *points) const = 0;
    virtual Volume volume_for_node(int node, const Point *points) const = 0;
};

} /* namespace os2cx */

#endif
