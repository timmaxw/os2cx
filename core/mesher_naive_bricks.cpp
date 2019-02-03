#include "mesher_naive_bricks.hpp"

#include <limits>

namespace os2cx {

Node3 mk_node(double x, double y, double z) {
    Node3 node;
    node.point = Point(x, y, z);
    return node;
}

Mesh3 mesher_naive_bricks(
    const Plc3 &plc,
    double max_tet_volume
) {
    (void)plc;
    (void)max_tet_volume;

    Mesh3 mesh;
    mesh.elements.reserve(1);
    mesh.nodes.reserve(8);

    Element3 element;
    element.type = ElementType::C3D8;

    element.nodes[0] = mesh.nodes.key_end();
    mesh.nodes.push_back(mk_node(-0.50, -0.05, -0.05));

    element.nodes[1] = mesh.nodes.key_end();
    mesh.nodes.push_back(mk_node(+0.50, -0.05, -0.05));

    element.nodes[2] = mesh.nodes.key_end();
    mesh.nodes.push_back(mk_node(+0.50, +0.05, -0.05));

    element.nodes[3] = mesh.nodes.key_end();
    mesh.nodes.push_back(mk_node(-0.50, +0.05, -0.05));

    element.nodes[4] = mesh.nodes.key_end();
    mesh.nodes.push_back(mk_node(-0.50, -0.05, +0.05));

    element.nodes[5] = mesh.nodes.key_end();
    mesh.nodes.push_back(mk_node(+0.50, -0.05, +0.05));

    element.nodes[6] = mesh.nodes.key_end();
    mesh.nodes.push_back(mk_node(+0.50, +0.05, +0.05));

    element.nodes[7] = mesh.nodes.key_end();
    mesh.nodes.push_back(mk_node(-0.50, +0.05, +0.05));

    mesh.elements.push_back(element);

    return mesh;
}

} /* namespace os2cx */
