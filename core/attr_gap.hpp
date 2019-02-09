#ifndef OS2CX_ATTR_GAP_HPP_
#define OS2CX_ATTR_GAP_HPP_

namespace os2cx {

class Gap {
public:
    class Pair {
    public:
        NodeId nodes[2];
        Vector gap;
    };
    std::vector<Pair> pairs;
};

Gap compute_gap_from_face_sets(
    const Mesh3 &mesh,
    const FaceSet &face_set_1,
    const FaceSet &face_set_2,
    double max_dist
);

} /* namespace os2cx */

#endif
