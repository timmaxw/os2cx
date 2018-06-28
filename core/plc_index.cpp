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

Length Plc3Index::approx_scale() const {
    double size = 0;
    size = std::max(size, i->tree.bbox().xmax());
    size = std::max(size, -i->tree.bbox().xmin());
    size = std::max(size, i->tree.bbox().ymax());
    size = std::max(size, -i->tree.bbox().ymin());
    size = std::max(size, i->tree.bbox().zmax());
    size = std::max(size, -i->tree.bbox().zmin());
    return Length(size);
}

Plc3::VolumeId Plc3Index::volume_containing_point(Point point) const {
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

Plc3::SurfaceId Plc3Index::surface_closest_to_point(Point point) const {
    CGAL::Point_3<KS> point2(point.x, point.y, point.z);
    PlcAabbTree::Point_and_primitive_id hit =
        i->tree.closest_point_and_primitive(point2);
    return hit.second.first;
}

} /* namespace os2cx */
