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

    ASSERT_EQ(4, mesh.nodes.size());

    const Node3 &n1 = mesh.nodes[NodeId::from_int(1)];
    EXPECT_EQ(0, n1.point.x);
    EXPECT_EQ(0, n1.point.y);
    EXPECT_FLOAT_EQ(0.1, n1.point.z);

    const Node3 &n2 = mesh.nodes[NodeId::from_int(2)];
    EXPECT_EQ(-1, n2.point.x);
    EXPECT_EQ(0, n2.point.y);
    EXPECT_FLOAT_EQ(n2.point.z, 0.2);

    const Node3 &n3 = mesh.nodes[NodeId::from_int(3)];
    EXPECT_EQ(0, n3.point.x);
    EXPECT_EQ(1, n3.point.y);
    EXPECT_FLOAT_EQ(-0.3, n3.point.z);

    ASSERT_EQ(1, mesh.elements.size());
    const Element3 &e = mesh.elements[ElementId::from_int(1)];
    EXPECT_EQ(ElementType::C3D4, e.type);
    EXPECT_EQ(1, e.nodes[0].to_int());
    EXPECT_EQ(2, e.nodes[1].to_int());
    EXPECT_EQ(3, e.nodes[2].to_int());
    EXPECT_EQ(4, e.nodes[3].to_int());
}

} /* namespace os2cx */
