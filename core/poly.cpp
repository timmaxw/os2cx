#include "poly.internal.hpp"

#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

namespace os2cx {

Poly3 Poly3::from_box(const Box &box) {
    return Poly3::from_boxes({box}, {false});
}

Poly3 Poly3::from_boxes(
    const std::vector<Box> &boxes,
    const std::vector<bool> &invert
) {
    assert(boxes.size() == invert.size());
    class BoxMaker : public CGAL::Modifier_base<CgalPolyhedron3::HalfedgeDS> {
    public:
        void operator()(CgalPolyhedron3::HalfedgeDS &hds) {
            for (int i = 0; i < (int)boxes.size(); ++i) {
                const Box &box = boxes[i];
                bool invert_box = invert[i];

                CGAL::Polyhedron_incremental_builder_3<
                    CgalPolyhedron3::HalfedgeDS> b(hds, true);

                b.begin_surface(8, 6, 12);
                b.add_vertex(CGAL::Point_3<K>(box.xl, box.yl, box.zh));
                b.add_vertex(CGAL::Point_3<K>(box.xh, box.yl, box.zh));
                b.add_vertex(CGAL::Point_3<K>(box.xh, box.yh, box.zh));
                b.add_vertex(CGAL::Point_3<K>(box.xl, box.yh, box.zl));
                b.add_vertex(CGAL::Point_3<K>(box.xl, box.yl, box.zl));
                b.add_vertex(CGAL::Point_3<K>(box.xh, box.yl, box.zl));
                b.add_vertex(CGAL::Point_3<K>(box.xh, box.yh, box.zl));
                b.add_vertex(CGAL::Point_3<K>(box.xl, box.yh, box.zh));

                int facets[12][3] = {
                    {4, 5, 1}, {0, 4, 1}, {0, 7, 4}, {4, 7, 3},
                    {0, 1, 2}, {7, 0, 2}, {3, 6, 4}, {4, 6, 5},
                    {5, 6, 2}, {1, 5, 2}, {7, 2, 3}, {3, 2, 6}
                };

                for (int i = 0; i < 12; ++i) {
                    b.begin_facet();
                    b.add_vertex_to_facet(facets[i][0]);
                    if (!invert_box) {
                        b.add_vertex_to_facet(facets[i][1]);
                        b.add_vertex_to_facet(facets[i][2]);
                    } else {
                        b.add_vertex_to_facet(facets[i][2]);
                        b.add_vertex_to_facet(facets[i][1]);
                    }
                    b.end_facet();
                }
                b.end_surface();
            }
        }
        std::vector<Box> boxes;
        std::vector<bool> invert;
    };
    BoxMaker bm;
    bm.boxes = boxes;
    bm.invert = invert;

    Poly3 r;
    r.i.reset(new Poly3Internal);
    r.i->p.delegate(bm);
    return r;
}

Poly3::Poly3() { }
Poly3::Poly3(Poly3 &&other) : i(std::move(other.i)) { }
Poly3::~Poly3() { }
Poly3 &Poly3::operator=(Poly3 &&other) {
    i = std::move(other.i);
    return *this;
}

Poly3 read_poly3_off(std::istream &stream) {
    Poly3 poly3;
    poly3.i.reset(new Poly3Internal);
    if (!(stream >> poly3.i->p)) {
        throw PolyIoError("OFF file read failed");
    }

    if (!CGAL::Polygon_mesh_processing::triangulate_faces(poly3.i->p)) {
        throw PolyIoError("OFF file cannot triangulate faces");
    }

    return poly3;
}

void write_poly3_off(std::ostream &stream, const Poly3 &poly) {
    stream << poly.i->p;
}

void write_poly3_stl_text(std::ostream &stream, const Poly3 &poly) {
    stream << "solid object\n";
    for (auto it = poly.i->p.facets_begin();
              it != poly.i->p.facets_end();
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

