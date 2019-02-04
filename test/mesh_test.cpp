#include <gtest/gtest.h>

#include "mesh.hpp"

namespace os2cx {

Mesh3 make_example_mesh(ElementType type, AffineTransform transform) {
    Mesh3 mesh;
    const ElementTypeShape &shape = element_type_shape(type);
    Element3 element;
    element.type = type;
    for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
        Node3 node;
        node.point = transform.apply(shape.vertices[i].uvw);
        element.nodes[i] = mesh.nodes.key_end();
        mesh.nodes.push_back(node);
    }
    mesh.elements.push_back(element);
    return mesh;
}

TEST(MeshTest, VolumeC3D8) {
    Mesh3 mesh = make_example_mesh(
        ElementType::C3D8,
        AffineTransform(Matrix::identity()));
    EXPECT_FLOAT_EQ(8.0, mesh.volume(*mesh.elements.begin()));
}

TEST(MeshTest, VolumeC3D20) {
    Mesh3 mesh = make_example_mesh(
        ElementType::C3D20,
        AffineTransform(Matrix::identity()));
    EXPECT_FLOAT_EQ(8.0, mesh.volume(*mesh.elements.begin()));
}

TEST(MeshTest, VolumeC3D4) {
    Mesh3 mesh = make_example_mesh(
        ElementType::C3D4,
        AffineTransform(Matrix::identity()));
    EXPECT_FLOAT_EQ(1/6.0, mesh.volume(*mesh.elements.begin()));
}

TEST(MeshTest, VolumeC3D10) {
    Mesh3 mesh = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::identity()));
    EXPECT_FLOAT_EQ(1/6.0, mesh.volume(*mesh.elements.begin()));
}

TEST(MeshTest, VolumeTransform) {
    Mesh3 mesh1 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::scale(3, 4, 5)));
    EXPECT_FLOAT_EQ(1/6.0*60, mesh1.volume(*mesh1.elements.begin()));

    Mesh3 mesh2 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::rotation(0, 1.23)));
    EXPECT_FLOAT_EQ(1/6.0, mesh2.volume(*mesh2.elements.begin()));
}

TEST(MeshTest, AreaRectangle4) {
    Mesh3 mesh = make_example_mesh(
        ElementType::C3D8,
        AffineTransform(Matrix::identity()));
    Vector area = mesh.oriented_area(*mesh.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area.x);
    EXPECT_FLOAT_EQ(0, area.y);
    EXPECT_FLOAT_EQ(-4.0, area.z);
}

TEST(MeshTest, AreaRectangle8) {
    Mesh3 mesh = make_example_mesh(
        ElementType::C3D20,
        AffineTransform(Matrix::identity()));
    Vector area = mesh.oriented_area(*mesh.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area.x);
    EXPECT_FLOAT_EQ(0, area.y);
    EXPECT_FLOAT_EQ(-4.0, area.z);
}

TEST(MeshTest, AreaTriangle3) {
    Mesh3 mesh = make_example_mesh(
        ElementType::C3D4,
        AffineTransform(Matrix::identity()));
    Vector area = mesh.oriented_area(*mesh.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area.x);
    EXPECT_FLOAT_EQ(0, area.y);
    EXPECT_FLOAT_EQ(-1/2.0, area.z);
}

TEST(MeshTest, AreaTriangle6) {
    Mesh3 mesh = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::identity()));
    Vector area = mesh.oriented_area(*mesh.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area.x);
    EXPECT_FLOAT_EQ(0, area.y);
    EXPECT_FLOAT_EQ(-1/2.0, area.z);
}

TEST(MeshTest, AreaTransform) {
    Mesh3 mesh1 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::scale(3, 4, 5)));
    Vector area1 = mesh1.oriented_area(*mesh1.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area1.x);
    EXPECT_FLOAT_EQ(0, area1.y);
    EXPECT_FLOAT_EQ(-1/2.0*12, area1.z);

    Mesh3 mesh2 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::rotation(0, 1.23)));
    Vector area2 = mesh2.oriented_area(*mesh2.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area2.x);
    EXPECT_FLOAT_EQ(-1/2.0*-sin(1.23), area2.y);
    EXPECT_FLOAT_EQ(-1/2.0*cos(1.23), area2.z);
}

} /* namespace os2cx */
