#include <math.h>

#include <sstream>

#include <gtest/gtest.h>

#include "calculix_inp_read.hpp"

namespace os2cx {

TEST(CalculixReadTest, ReadCalculixNodesAndElements) {
    std::ostringstream ss;
    ss << "*Heading\n";
    ss << " this is a test file\n";
    ss << "*Node\n";
    ss << "1, 0, 0, 0.1\n";
    ss << "2, -1, 0, 0.2\n";
    ss << "3, 0, 1, -0.3\n";
    ss << "4, 10, 11, 12\n";
    ss << "*Element, type=C3D4\n";
    ss << "1, 1, 2, 3, 4\n";

    Mesh3 mesh;
    std::istringstream input(ss.str());
    read_calculix_nodes_and_elements(input, &mesh);

    assert(mesh.nodes.size() == 4);

    const Node3 &n1 = mesh.nodes[NodeId::from_int(1)];
    assert(n1.point.vector.x.val == 0);
    assert(n1.point.vector.y.val == 0);
    assert(fabs(n1.point.vector.z.val - 0.1) < 1e-10);

    const Node3 &n2 = mesh.nodes[NodeId::from_int(2)];
    assert(n2.point.vector.x.val == -1);
    assert(n2.point.vector.y.val == 0);
    assert(fabs(n2.point.vector.z.val - 0.2) < 1e-10);

    const Node3 &n3 = mesh.nodes[NodeId::from_int(3)];
    assert(n3.point.vector.x.val == 0);
    assert(n3.point.vector.y.val == 1);
    assert(fabs(n3.point.vector.z.val - (-0.3)) < 1e-10);

    assert(mesh.elements.size() == 1);
    const Element3 &e = mesh.elements[ElementId::from_int(1)];
    assert(e.type == ElementType::C3D4);
    assert(e.nodes[0].to_int() == 1);
    assert(e.nodes[1].to_int() == 2);
    assert(e.nodes[2].to_int() == 3);
    assert(e.nodes[3].to_int() == 4);
}

} /* namespace os2cx */
