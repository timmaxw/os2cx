#include "region.internal.hpp"

namespace os2cx {

Region3 Region3::box(
    double x1, double y1, double z1,
    double x2, double y2, double z2
) {
    class BoxMaker : public CGAL::Modifier_base<CgalPolyhedron3::HalfedgeDS> {
    public:
        void operator()(CgalPolyhedron3::HalfedgeDS &hds) {
            CGAL::Polyhedron_incremental_builder_3<
                CgalPolyhedron3::HalfedgeDS> b(hds, true);
            b.begin_surface(8, 6, 12);
            b.add_vertex(CGAL::Point_3<K>(x1, y1, z2));
            b.add_vertex(CGAL::Point_3<K>(x2, y1, z2));
            b.add_vertex(CGAL::Point_3<K>(x2, y2, z2));
            b.add_vertex(CGAL::Point_3<K>(x1, y2, z1));
            b.add_vertex(CGAL::Point_3<K>(x1, y1, z1));
            b.add_vertex(CGAL::Point_3<K>(x2, y1, z1));
            b.add_vertex(CGAL::Point_3<K>(x2, y2, z1));
            b.add_vertex(CGAL::Point_3<K>(x1, y2, z2));
            int facets[12][3] = {
                {4, 5, 1}, {0, 4, 1}, {0, 7, 4}, {4, 7, 3},
                {0, 1, 2}, {7, 0, 2}, {3, 6, 4}, {4, 6, 5},
                {5, 6, 2}, {1, 5, 2}, {7, 2, 3}, {3, 2, 6}
            };
            for (int i = 0; i < 12; ++i) {
                b.begin_facet();
                b.add_vertex_to_facet(facets[i][0]);
                b.add_vertex_to_facet(facets[i][1]);
                b.add_vertex_to_facet(facets[i][2]);
                b.end_facet();
            }
            b.end_surface();
        }
        double x1, y1, z1, x2, y2, z2;
    };
    BoxMaker bm;
    bm.x1 = x1; bm.y1 = y1; bm.z1 = z1; bm.x2 = x2; bm.y2 = y2; bm.z2 = z2;

    Region3 r;
    r.i.reset(new Region3Internal);
    r.i->p.delegate(bm);
    return r;
}

Region3::Region3() { }
Region3::Region3(Region3 &&other) : i(std::move(other.i)) { }
Region3::~Region3() { }
Region3 &Region3::operator=(Region3 &&other) {
    i = std::move(other.i);
    return *this;
}

Region3 read_region_off(std::istream &stream) {
    Region3 region;
    region.i.reset(new Region3Internal);
    if (!(stream >> region.i->p)) {
        throw Region_IOError("OFF file read failed");
    }
    return std::move(region);
}

void write_region_off(std::ostream &stream, const Region3 &region) {
    stream << region.i->p;
}

void write_region_stl_text(
    std::ostream &stream,
    const Region3 &r
) {
    stream << "solid object\n";
    for (auto it = r.i->p.facets_begin();
              it != r.i->p.facets_end();
              ++it) {
        os2cx::CgalPolyhedron3::Halfedge_around_facet_circulator jt =
            it->facet_begin();
        CGAL::Point_3<K> p1 = jt->vertex()->point();
        ++jt;
        CGAL::Point_3<K> p2 = jt->vertex()->point();
        ++jt;
        CGAL::Point_3<K> p3 = jt->vertex()->point();
        CGAL::Vector_3<K> n = CGAL::unit_normal(p1, p2, p3);
        stream
            << "facet normal " << n.x() << ' ' << n.y() << ' ' << n.z() << '\n'
            << "outer loop\n"
            << "vertex " << p1.x() << ' ' << p1.y() << ' ' << p1.z() << '\n'
            << "vertex " << p2.x() << ' ' << p2.y() << ' ' << p2.z() << '\n'
            << "vertex " << p3.x() << ' ' << p3.y() << ' ' << p3.z() << '\n'
            << "endloop\nendfacet\n";
    }
    stream << "endsolid object\n";
}

} /* namespace os2cx */

