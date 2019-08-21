#include <gtest/gtest.h>

#include "mesh_index.hpp"

namespace os2cx {

TEST(MeshIndexTest, Mesh3Index) {
    Mesh3 mesh;
    NodeId n[5];
    for (int i = 0; i < 5; ++i) {
        n[i] = mesh.nodes.key_end();
        mesh.nodes.push_back(Node3());
    }
    AttrBitset attrs;
    attrs.set(attr_bit_solid());

    mesh.elements.push_back(Element3 {
        ElementType::C3D4,
        {n[0], n[1], n[2], n[3]},
        attrs,
        {attrs, attrs, attrs, attrs}
    });
    mesh.elements.push_back(Element3 {
        ElementType::C3D4,
        {n[0], n[2], n[1], n[4]},
        attrs,
        {attrs, attrs, attrs, attrs}
    });

    Mesh3Index index(mesh);

    for (int i = 0; i < 2; ++i) {
        ElementId eid = ElementId::from_int(1 + i);
        ElementId other_eid = ElementId::from_int(1 + !i);
        for (int face = 0; face < 4; ++face) {
            auto match = index.matching_face(FaceId { eid, face });
            if (face == 0) {
                EXPECT_EQ(other_eid, match.element_id);
                EXPECT_EQ(0, match.face);
            } else {
                EXPECT_EQ(FaceId::invalid(), match);
            }
        }
    }
}

} /* namespace os2cx */
