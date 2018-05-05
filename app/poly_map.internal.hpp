#ifndef OS2CX_POLY_MAP_INTERNAL_HPP_
#define OS2CX_POLY_MAP_INTERNAL_HPP_

#include "poly_map.hpp"

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Nef_polyhedron_3.h>

#include "poly.internal.hpp"

namespace os2cx {

typedef CGAL::Exact_predicates_exact_constructions_kernel KE;
typedef CGAL::Nef_polyhedron_3<KE> CgalNef3;

class DebugNef {
public:
    DebugNef(const os2cx::CgalNef3 &n);

    void dump(std::ostream &stream) const;

    const os2cx::CgalNef3 &nef;
    CGAL::Inverse_index<os2cx::CgalNef3::Vertex_const_iterator>
        vertex_index;
    CGAL::Inverse_index<os2cx::CgalNef3::Halfedge_const_iterator>
        halfedge_index;
    CGAL::Inverse_index<os2cx::CgalNef3::Halffacet_const_iterator>
        halffacet_index;
    CGAL::Inverse_index<os2cx::CgalNef3::Volume_const_iterator>
        volume_index;
};

class Poly3MapInternal {
public:
    os2cx::CgalNef3 nef;
    CGAL::Inverse_index<os2cx::CgalNef3::Vertex_const_handle> vertex_from_nef;
    std::vector<os2cx::CgalNef3::Vertex_const_handle> vertex_to_nef;
    CGAL::Inverse_index<os2cx::CgalNef3::Volume_const_handle> volume_from_nef;
    std::vector<os2cx::CgalNef3::Volume_const_handle> volume_to_nef;
};

} /* namespace os2cx */

#endif

