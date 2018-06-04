#include "beacon.hpp"

#include "poly.internal.hpp"

namespace os2cx {

AffineTransform recover_beacon(const Poly3 &beacon) {
    const os2cx::CgalPolyhedron3 &p = beacon.i->p;

    /* Make sure we have the right number of vertices and each vertex has the
    right degree (two degree-3 vertices, three degree-4 vertices) */
    std::vector<Point> deg3, deg4;
    for (auto it = p.vertices_begin(); it != p.vertices_end(); ++it) {
        Point point;
        point.vector.x.val = it->point().x();
        point.vector.y.val = it->point().y();
        point.vector.z.val = it->point().z();
        if (it->vertex_degree() == 3) {
            deg3.push_back(point);
        } else if (it->vertex_degree() == 4) {
            deg4.push_back(point);
        } else {
            throw BeaconError();
        }
    }
    if (deg3.size() != 2 || deg4.size() != 3) {
        throw BeaconError();
    }

    double concave_coords[3] = {0.1, 0.2, 0.3};

    /* One of the degree-3 vertices is the "origin" vertex and the other is the
    "concave" vertex. We can tell them apart because the "concave" vertex is
    closer to the average of the degree-4 vertices. */
    Point deg4_avg((deg4[0].vector + deg4[1].vector + deg4[2].vector) / 3.0);
    Point origin, concave;
    if ((deg3[1] - deg3[0]).dot(deg4_avg - deg3[0]).val > 0) {
        origin = deg3[0];
        concave = deg3[1];
    } else {
        concave = deg3[0];
        origin = deg3[1];
    }

    /* The degree-4 vertices are the "corner_x", "corner_y", and "corner_z"
    vertices. We can figure out which is which by comparing them to the
    "concave" vertex. */
    Point corner_xyz[3];
    bool corner_found[3] = {false, false, false};
    for (Point candidate : deg4) {
        Length distance = (candidate - origin).magnitude();
        if (distance.val == 0) {
            throw BeaconError();
        }
        PureVector direction = (candidate - origin) / distance;
        double a = ((concave - origin).dot(direction) / distance).val;
        for (int i = 0; i < 3; ++i) {
            if (fabs(a - concave_coords[i]) < 1e-3) {
                if (corner_found[i]) {
                    throw BeaconError();
                }
                corner_xyz[i] = candidate;
                corner_found[i] = true;
                break;
            }
        }
    }
    if (!corner_found[0] || !corner_found[1] || !corner_found[2]) {
        throw BeaconError();
    }

    /* Once we know which vertex is which, it's easy to reconstruct the affine
    transformation that was applied to the beacon */
    AffineTransform transform;
    transform.translation = origin.vector;
    for (int i = 0; i < 3; ++i) {
        transform.matrix[i] = corner_xyz[i] - origin;
    }
    return transform;
}

} /* namespace os2cx */
