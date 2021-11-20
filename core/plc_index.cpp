#include "plc_index.hpp"

#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Simple_cartesian.h>

namespace os2cx {

typedef CGAL::Simple_cartesian<double> KS;

/* This is a model of the concept CGAL::AABBPrimitiveWithSharedData */
class PlcAabbPrimitive {
public:
    typedef CGAL::Point_3<KS> Point;
    typedef CGAL::Triangle_3<KS> Datum;
    typedef std::pair<Plc3::SurfaceId, int> Id;
    typedef const Plc3 *Shared_data;

    static Shared_data construct_shared_data(const Plc3 *pm) {
        return pm;
    }

    PlcAabbPrimitive() { }

    template<class Iterator>
    PlcAabbPrimitive(Iterator it, const Plc3 *) : i(*it) { }

    Datum datum(const Plc3 *rm) const {
        const Plc3::Surface::Triangle &tri =
            rm->surfaces[i.first].triangles[i.second];
        return CGAL::Triangle_3<KS>(
            get_point(rm, tri.vertices[0]),
            get_point(rm, tri.vertices[1]),
            get_point(rm, tri.vertices[2]));
    }

    Id id() const {
        return i;
    }

    Point reference_point(const Plc3 *rm) const {
        const Plc3::Surface::Triangle &tri =
            rm->surfaces[i.first].triangles[i.second];
        return get_point(rm, tri.vertices[0]);
    }

private:
    Point get_point(const Plc3 *rm, Plc3::VertexId vid) const {
        return CGAL::Point_3<KS>(
            rm->vertices[vid].point.x,
            rm->vertices[vid].point.y,
            rm->vertices[vid].point.z);
    }

    Id i;
};

class PlcAabbPrimitiveIterator {
public:
    bool operator!=(const PlcAabbPrimitiveIterator &other) {
        return pm != other.pm || pair != other.pair;
    }
    PlcAabbPrimitiveIterator &operator++() {
        pair.second += 1;
        while (pair.first != static_cast<int>(pm->surfaces.size()) &&
                pair.second == static_cast<int>(
                    pm->surfaces[pair.first].triangles.size())) {
            ++pair.first;
            pair.second = 0;
        }
        return *this;
    }
    PlcAabbPrimitiveIterator operator++(int) {
        PlcAabbPrimitiveIterator copy = *this;
        ++*this;
        return copy;
    }
    const std::pair<Plc3::SurfaceId, int> &operator*() const {
        return pair;
    }
    const std::pair<Plc3::SurfaceId, int> *operator->() const {
        return &pair;
    }

    const Plc3 *pm;
    std::pair<Plc3::SurfaceId, int> pair;
};

typedef CGAL::AABB_traits<KS, PlcAabbPrimitive> PlcAabbTraits;
typedef CGAL::AABB_tree<PlcAabbTraits> PlcAabbTree;

class Plc3IndexInternal {
public:
    PlcAabbTree tree;
};

Plc3Index::Plc3Index(const Plc3 *plc_) : plc(plc_)
{
    i.reset(new Plc3IndexInternal);
    PlcAabbPrimitiveIterator begin {plc, {0, 0}};
    PlcAabbPrimitiveIterator end {plc, {plc->surfaces.size(), 0}};
    i->tree.rebuild(begin, end, plc);
    i->tree.accelerate_distance_queries();
}

Plc3Index::~Plc3Index() { }

Plc3::VolumeId Plc3Index::volume_containing_point(Point point) const {
    /* Shoot a ray in a random direction, until it hits a triangle. Then ask
    which volume is on the side of the triangle that was hit. */

    CGAL::Random_points_on_sphere_3<CGAL::Point_3<KS> > random;
    CGAL::Vector_3<KS> direction(CGAL::ORIGIN, *random);
    CGAL::Ray_3<KS> ray(
        CGAL::Point_3<KS>(point.x, point.y, point.z),
        direction);

    boost::optional<PlcAabbPrimitive::Id> hit =
        i->tree.first_intersected_primitive(ray);
    if (!hit) return plc->volume_outside;

    const Plc3::Surface &surface = plc->surfaces[hit->first];
    const Plc3::Surface::Triangle &tri = surface.triangles[hit->second];

    Vector normal = triangle_normal(
        plc->vertices[tri.vertices[0]].point,
        plc->vertices[tri.vertices[1]].point,
        plc->vertices[tri.vertices[2]].point);
    double dot =
        direction.x() * normal.x +
        direction.y() * normal.y +
        direction.z() * normal.z;
    /* The normal vector points into 'surface.volumes[0]' */
    return surface.volumes[(dot > 0) ? 1 : 0];
}

static const double epsilon = 1e-9;

Plc3::SurfaceId Plc3Index::surface_containing_point(Point point) const {
    CGAL::Point_3<KS> point2(point.x, point.y, point.z);
    PlcAabbTree::Point_and_primitive_id hit =
        i->tree.closest_point_and_primitive(point2);
    double sq_dist = CGAL::squared_distance(hit.first, point2);
    if (sq_dist > epsilon * epsilon) {
        return -1;
    }
    return hit.second.first;
}

Plc3::VertexId Plc3Index::vertex_at_point(Point point) const {
    /* Find the nearest triangle to the given point. Then check whether any
    vertex of that triangle coincides with the given point. */
    CGAL::Point_3<KS> point2(point.x, point.y, point.z);
    PlcAabbTree::Point_and_primitive_id hit =
        i->tree.closest_point_and_primitive(point2);
    Plc3::Surface::Triangle triangle =
        plc->surfaces[hit.second.first].triangles[hit.second.second];
    for (int j = 0; j < 3; ++j) {
        Plc3::VertexId vertex = triangle.vertices[j];
        double dist = (point - plc->vertices[vertex].point).magnitude();
        if (dist < epsilon) {
            return vertex;
        }
    }
    return -1;
}


} /* namespace os2cx */
