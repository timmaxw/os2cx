#include "poly_map_index.hpp"

#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Simple_cartesian.h>

namespace os2cx {

typedef CGAL::Simple_cartesian<double> KS;

/* This is a model of the concept CGAL::AABBPrimitiveWithSharedData */
class CgalAabbPrimitive {
public:
    typedef CGAL::Point_3<KS> Point;
    typedef CGAL::Triangle_3<KS> Datum;
    typedef std::pair<Poly3Map::SurfaceId, int> Id;
    typedef const Poly3Map *Shared_data;

    static Shared_data construct_shared_data(const Poly3Map *pm) {
        return pm;
    }

    CgalAabbPrimitive() { }

    template<class Iterator>
    CgalAabbPrimitive(Iterator it, const Poly3Map *) : i(*it) { }

    Datum datum(const Poly3Map *rm) const {
        const Poly3Map::Surface::Triangle &tri =
            rm->surfaces[i.first].triangles[i.second];
        return CGAL::Triangle_3<KS>(
            get_point(rm, tri.vertices[0]),
            get_point(rm, tri.vertices[1]),
            get_point(rm, tri.vertices[2]));
    }

    Id id() const {
        return i;
    }

    Point reference_point(const Poly3Map *rm) const {
        const Poly3Map::Surface::Triangle &tri =
            rm->surfaces[i.first].triangles[i.second];
        return get_point(rm, tri.vertices[0]);
    }

private:
    Point get_point(const Poly3Map *rm, Poly3Map::VertexId vid) const {
        return CGAL::Point_3<KS>(
            rm->vertices[vid].point.vector.x.val,
            rm->vertices[vid].point.vector.y.val,
            rm->vertices[vid].point.vector.z.val);
    }

    Id i;
};

class CgalAabbPrimitiveIterator {
public:
    bool operator!=(const CgalAabbPrimitiveIterator &other) {
        return pm != other.pm || pair != other.pair;
    }
    CgalAabbPrimitiveIterator &operator++() {
        pair.second += 1;
        while (pair.first != static_cast<int>(pm->surfaces.size()) &&
                pair.second == static_cast<int>(
                    pm->surfaces[pair.first].triangles.size())) {
            ++pair.first;
            pair.second = 0;
        }
        return *this;
    }
    CgalAabbPrimitiveIterator operator++(int) {
        CgalAabbPrimitiveIterator copy = *this;
        ++*this;
        return copy;
    }
    const std::pair<Poly3Map::SurfaceId, int> &operator*() const {
        return pair;
    }
    const std::pair<Poly3Map::SurfaceId, int> *operator->() const {
        return &pair;
    }

    const Poly3Map *pm;
    std::pair<Poly3Map::SurfaceId, int> pair;
};

typedef CGAL::AABB_traits<KS, os2cx::CgalAabbPrimitive> CgalAabbTraits;
typedef CGAL::AABB_tree<os2cx::CgalAabbTraits> CgalAabbTree;

class Poly3MapIndexInternal {
public:
    CgalAabbTree tree;
};

Poly3MapIndex::Poly3MapIndex(const Poly3Map &pm) :
        poly3_map(&pm)
{
    i.reset(new Poly3MapIndexInternal);
    CgalAabbPrimitiveIterator begin {&pm, {0, 0}};
    CgalAabbPrimitiveIterator end {&pm, {pm.surfaces.size(), 0}};
    i->tree.rebuild(begin, end, &pm);
    i->tree.accelerate_distance_queries();
}

Poly3MapIndex::~Poly3MapIndex() { }

Length Poly3MapIndex::approx_scale() const {
    double size = 0;
    size = std::max(size, i->tree.bbox().xmax());
    size = std::max(size, -i->tree.bbox().xmin());
    size = std::max(size, i->tree.bbox().ymax());
    size = std::max(size, -i->tree.bbox().ymin());
    size = std::max(size, i->tree.bbox().zmax());
    size = std::max(size, -i->tree.bbox().zmin());
    return Length(size);
}

Poly3Map::VolumeId Poly3MapIndex::volume_containing_point(
    Point point
) const {
    CGAL::Random_points_on_sphere_3<CGAL::Point_3<KS> > random;
    CGAL::Vector_3<KS> direction(CGAL::ORIGIN, *random);
    CGAL::Ray_3<KS> ray(
        CGAL::Point_3<KS>(
            point.vector.x.val,
            point.vector.y.val,
            point.vector.z.val),
        direction);

    boost::optional<os2cx::CgalAabbPrimitive::Id> hit =
        i->tree.first_intersected_primitive(ray);
    if (!hit) return 0;

    const Poly3Map::Surface &surface = poly3_map->surfaces[hit->first];
    const Poly3Map::Surface::Triangle &tri = surface.triangles[hit->second];

    double dot =
        direction.x() * tri.normal.x.val +
        direction.y() * tri.normal.y.val +
        direction.z() * tri.normal.z.val;
    /* The normal vector points into 'surface.volumes[0]' */
    return surface.volumes[(dot > 0) ? 1 : 0];
}

Poly3Map::SurfaceId Poly3MapIndex::surface_closest_to_point(
    Point point
) const {
    CGAL::Point_3<KS> point2(
        point.vector.x.val,
        point.vector.y.val,
        point.vector.z.val);
    os2cx::CgalAabbTree::Point_and_primitive_id hit =
        i->tree.closest_point_and_primitive(point2);
    return hit.second.first;
}

} /* namespace os2cx */

