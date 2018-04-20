#ifndef OS2CX_REGION_INTERNAL_HPP_
#define OS2CX_REGION_INTERNAL_HPP_

#include "region.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>

namespace os2cx {

/* To minimize confusion around what's part of CGAL, what's part of OS2CX, and
what's in the interface between them, we use the following naming conventions:
  - The CGAL:: namespace prefix is always written out explicitly.
  - When CGAL is in scope, the os2cx:: namespace prefix is also written out
    explicitly.
  - Types that are defined in os2cx but only used for interactions with CGAL are
    in the namespace os2cx but prefixed with CGAL_*.
  - When CGAL types are extended with OS2CX-specific fields, those fields are
    prefixed with os2cx_*.
  - As an exception, the namespace prefixes for os2cx::K and os2cx::KE are
    usually omitted.
*/

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K> CgalPolyhedron3;

class Region3Internal {
public:
    template<typename... Args>
    Region3Internal(Args&&... args) : p(args...) { }
    os2cx::CgalPolyhedron3 p;
};

template<class K1, class I1, class K2, class I2>
void convert_polyhedron(
        const CGAL::Polyhedron_3<K1, I1> &input,
        CGAL::Polyhedron_3<K2, I2> *output) {
    typedef CGAL::Polyhedron_3<K1, I1> P1;
    typedef CGAL::Polyhedron_3<K2, I2> P2;
    class Converter : public CGAL::Modifier_base<typename P2::HalfedgeDS> {
    public:
        void operator()(typename P2::HalfedgeDS& hds) {
            CGAL::Polyhedron_incremental_builder_3<typename P2::HalfedgeDS>
                b(hds, true);
            b.begin_surface(
                input->size_of_vertices(),
                input->size_of_facets(),
                input->size_of_halfedges());
            CGAL::Inverse_index<typename P1::Vertex_const_iterator>
                vertex_index(input->vertices_begin(), input->vertices_end());
            for (auto it = input->vertices_begin();
                    it != input->vertices_end(); ++it) {
                const CGAL::Point_3<K1> &p1(it->point());
                CGAL::NT_converter<typename K1::RT, typename K2::RT>
                    nt_converter;
                CGAL::Point_3<K2> p2(
                    nt_converter(p1.x()),
                    nt_converter(p1.y()),
                    nt_converter(p1.z()));
                b.add_vertex(p2);
            }
            for (auto it = input->facets_begin();
                    it != input->facets_end(); ++it) {
                b.begin_facet();
                for (auto jt = it->facet_begin();;) {
                    typename P1::Vertex_const_handle vertex_h = jt->vertex();
                    typename P1::Vertex_const_iterator vertex_it(vertex_h);
                    size_t vertex_i = vertex_index[vertex_it];
                    b.add_vertex_to_facet(vertex_i);
                    ++jt;
                    if (jt == it->facet_begin()) break;
                }
                b.end_facet();
            }
            b.end_surface();
        }
        const P1 *input;
    } converter;
    converter.input = &input;
    output->delegate(converter);
}

} /* namespace os2cx */

#endif
