#include <gtest/gtest.h>

#include "mesher_naive_bricks.hpp"
#include "plc_nef_to_plc.hpp"

namespace os2cx {

/* Comparison operator so we can make sets of boxes */
class BoxLess {
public:
    bool operator()(const Box &box1, const Box &box2) const {
        if (box1.xl < box2.xl) return true;
        if (box1.xl > box2.xl) return false;
        if (box1.yl < box2.yl) return true;
        if (box1.yl > box2.yl) return false;
        if (box1.zl < box2.zl) return true;
        if (box1.zl > box2.zl) return false;
        if (box1.xh < box2.xh) return true;
        if (box1.xh > box2.xh) return false;
        if (box1.yh < box2.yh) return true;
        if (box1.yh > box2.yh) return false;
        if (box1.zh < box2.zh) return true;
        if (box1.zh > box2.zh) return false;
        return false;
    }
};

enum BoxTransform {
    IDENTITY = 0,
    PERMUTE_YZX = 1,
    PERMUTE_ZXY = 2,
    PERMUTE_ZYX = 3,
    PERMUTE_YXZ = 4,
    PERMUTE_XZY = 5,
    PERMUTE_MASK = 7,
    INVERT_X = 8,
    INVERT_Y = 16,
    INVERT_Z = 32
};

Box box_transform(const Box &b, int t) {
    Box b2;
    switch (t & PERMUTE_MASK) {
    case IDENTITY:
        b2 = b;
        break;
    case PERMUTE_YZX:
        b2 = Box(b.yl, b.zl, b.xl, b.yh, b.zh, b.xh);
        break;
    case PERMUTE_ZXY:
        b2 = Box(b.zl, b.xl, b.yl, b.zh, b.xh, b.yh);
        break;
    case PERMUTE_ZYX:
        b2 = Box(b.zl, b.yl, b.xl, b.zh, b.yh, b.xh);
        break;
    case PERMUTE_YXZ:
        b2 = Box(b.yl, b.xl, b.zl, b.yh, b.xh, b.zh);
        break;
    case PERMUTE_XZY:
        b2 = Box(b.xl, b.zl, b.yl, b.xh, b.zh, b.yh);
        break;
    default: assert(false);
    }

    Box b3 = b2;
    if (t & INVERT_X) {
        b3.xl = -b2.xh;
        b3.xh = -b2.xl;
    }
    if (t & INVERT_Y) {
        b3.yl = -b2.yh;
        b3.yh = -b2.yl;
    }
    if (t & INVERT_Z) {
        b3.zl = -b2.zh;
        b3.zh = -b2.zl;
    }

    return b3;
}

Box element_c3d8_to_box(const Mesh3 &mesh, const Element3 &element) {
    Point p[8];
    for (int i = 0; i < 8; ++i) p[i] = mesh.nodes[element.nodes[i]].point;

    Box box(p[0].x, p[0].y, p[0].z, p[6].x, p[6].y, p[6].z);
    EXPECT_EQ(Point(box.xl, box.yl, box.zl), p[0]);
    EXPECT_EQ(Point(box.xh, box.yl, box.zl), p[1]);
    EXPECT_EQ(Point(box.xh, box.yh, box.zl), p[2]);
    EXPECT_EQ(Point(box.xl, box.yh, box.zl), p[3]);
    EXPECT_EQ(Point(box.xl, box.yl, box.zh), p[4]);
    EXPECT_EQ(Point(box.xh, box.yl, box.zh), p[5]);
    EXPECT_EQ(Point(box.xh, box.yh, box.zh), p[6]);
    EXPECT_EQ(Point(box.xl, box.yh, box.zh), p[7]);

    return box;
}

void try_mnb(
    const std::vector<Box> &boxes_in,
    const std::vector<Box> &boxes_out,
    int transform
) {
    PlcNef3 example = PlcNef3::empty();
    for (const Box &box : boxes_in) {
        Box box2 = box_transform(box, transform);
        example = example.binary_or(PlcNef3::from_poly(Poly3::from_box(box2)));
    }
    Plc3 plc = plc_nef_to_plc(example);

    Mesh3 mesh = mesher_naive_bricks(plc, 1e6);

    std::set<Box, BoxLess> expected_boxes;
    for (const Box &box : boxes_out) {
        Box box2 = box_transform(box, transform);
        expected_boxes.insert(box2);
    }
    std::set<Box, BoxLess> unmatched_boxes = expected_boxes;

    for (const Element3 &element : mesh.elements) {
        ASSERT_EQ(ElementType::C3D8, element.type);
        Box box = element_c3d8_to_box(mesh, element);
        if (expected_boxes.count(box)) {
            if (unmatched_boxes.count(box)) {
                unmatched_boxes.erase(box);
            } else {
                FAIL() << "duplicate box: " << box;
            }
        } else {
            FAIL() << "unexpected box: " << box;
        }
    }
    for (const Box &box : unmatched_boxes) {
        FAIL() << "expected, but did not get, box: " << box;
    }
}

TEST(MesherNaiveBricksTest, SingleBrick) {
    try_mnb(
        {Box(0, 0, 0, 1, 1, 1)},
        {Box(0, 0, 0, 1, 1, 1)},
        IDENTITY
    );
}

std::vector<Box> two_bricks_in({
    Box(0, 0, 0, 1, 1, 1),
    Box(2, 0, 0, 3, 1, 1)
});
std::vector<Box> two_bricks_out({
    Box(0, 0, 0, 1, 1, 1),
    Box(2, 0, 0, 3, 1, 1)
});
TEST(MesherNaiveBricksTest, TwoBricksX) {
    try_mnb(two_bricks_in, two_bricks_out, IDENTITY);
}
TEST(MesherNaiveBricksTest, TwoBricksY) {
    try_mnb(two_bricks_in, two_bricks_out, PERMUTE_YXZ);
}
TEST(MesherNaiveBricksTest, TwoBricksZ) {
    try_mnb(two_bricks_in, two_bricks_out, PERMUTE_ZYX);
}

TEST(MesherNaiveBricksTest, DiagonalBricks) {
    try_mnb(
        {Box(0, 0, 0, 1, 1, 1), Box(1, 1, 1, 2, 2, 2)},
        {Box(0, 0, 0, 1, 1, 1), Box(1, 1, 1, 2, 2, 2)},
        IDENTITY
    );
}

std::vector<Box> donut_in({
    Box(0, 0, 0, 3, 1, 1),
    Box(0, 0, 0, 1, 3, 1),
    Box(2, 0, 0, 3, 3, 1),
    Box(0, 2, 0, 3, 3, 1)
});
std::vector<Box> donut_out({
    Box(0, 0, 0, 1, 1, 1),
    Box(1, 0, 0, 2, 1, 1),
    Box(2, 0, 0, 3, 1, 1),
    Box(0, 1, 0, 1, 2, 1),
    Box(2, 1, 0, 3, 2, 1),
    Box(0, 2, 0, 1, 3, 1),
    Box(1, 2, 0, 2, 3, 1),
    Box(2, 2, 0, 3, 3, 1)
});
TEST(MesherNaiveBricksTest, DonutXY) {
    try_mnb(donut_in, donut_out, IDENTITY);
}
TEST(MesherNaiveBricksTest, DonutXZ) {
    try_mnb(donut_in, donut_out, PERMUTE_XZY);
}
TEST(MesherNaiveBricksTest, DonutYZ) {
    try_mnb(donut_in, donut_out, PERMUTE_ZYX);
}

std::vector<Box> elbow_in({
    Box(0, 0, 0, 2, 1, 1),
    Box(0, 0, 0, 1, 2, 1),
});
std::vector<Box> elbow_out({
    Box(0, 0, 0, 1, 1, 1),
    Box(1, 0, 0, 2, 1, 1),
    Box(0, 1, 0, 1, 2, 1),
});
TEST(MesherNaiveBricksTest, ElbowPXPY) {
    try_mnb(elbow_in, elbow_out, IDENTITY);
}
TEST(MesherNaiveBricksTest, ElbowNXPY) {
    try_mnb(elbow_in, elbow_out, INVERT_X);
}
TEST(MesherNaiveBricksTest, ElbowPXNY) {
    try_mnb(elbow_in, elbow_out, INVERT_Y);
}
TEST(MesherNaiveBricksTest, ElbowNXNY) {
    try_mnb(elbow_in, elbow_out, INVERT_X|INVERT_Y);
}
TEST(MesherNaiveBricksTest, ElbowPXPZ) {
    try_mnb(elbow_in, elbow_out, PERMUTE_XZY);
}
TEST(MesherNaiveBricksTest, ElbowNXPZ) {
    try_mnb(elbow_in, elbow_out, PERMUTE_XZY|INVERT_X);
}
TEST(MesherNaiveBricksTest, ElbowPXNZ) {
    try_mnb(elbow_in, elbow_out, PERMUTE_XZY|INVERT_Z);
}
TEST(MesherNaiveBricksTest, ElbowNXNZ) {
    try_mnb(elbow_in, elbow_out, PERMUTE_XZY|INVERT_X|INVERT_Z);
}
TEST(MesherNaiveBricksTest, ElbowPYPZ) {
    try_mnb(elbow_in, elbow_out, PERMUTE_YZX);
}
TEST(MesherNaiveBricksTest, ElbowNYPZ) {
    try_mnb(elbow_in, elbow_out, PERMUTE_YZX|INVERT_Y);
}
TEST(MesherNaiveBricksTest, ElbowPYNZ) {
    try_mnb(elbow_in, elbow_out, PERMUTE_YZX|INVERT_Z);
}
TEST(MesherNaiveBricksTest, ElbowNYNZ) {
    try_mnb(elbow_in, elbow_out, PERMUTE_YZX|INVERT_Y|INVERT_Z);
}

std::vector<Box> ibeam_in({
    Box(0, 0, 0, 3, 1, 1),
    Box(1, 0, 0, 2, 3, 1),
    Box(0, 2, 0, 3, 3, 1)
});
std::vector<Box> ibeam_out({
    Box(0, 0, 0, 1, 1, 1),
    Box(1, 0, 0, 2, 1, 1),
    Box(2, 0, 0, 3, 1, 1),
    Box(1, 1, 0, 2, 2, 1),
    Box(0, 2, 0, 1, 3, 1),
    Box(1, 2, 0, 2, 3, 1),
    Box(2, 2, 0, 3, 3, 1),
});
TEST(MesherNaiveBricksTest, IBeamXY) {
    try_mnb(ibeam_in, ibeam_out, IDENTITY);
}
TEST(MesherNaiveBricksTest, IBeamYZ) {
    try_mnb(ibeam_in, ibeam_out, PERMUTE_YZX);
}
TEST(MesherNaiveBricksTest, IBeamZX) {
    try_mnb(ibeam_in, ibeam_out, PERMUTE_ZXY);
}
TEST(MesherNaiveBricksTest, IBeamYX) {
    try_mnb(ibeam_in, ibeam_out, PERMUTE_YXZ);
}
TEST(MesherNaiveBricksTest, IBeamZY) {
    try_mnb(ibeam_in, ibeam_out, PERMUTE_ZYX);
}
TEST(MesherNaiveBricksTest, IBeamXZ) {
    try_mnb(ibeam_in, ibeam_out, PERMUTE_XZY);
}

} /* namespace os2cx */
