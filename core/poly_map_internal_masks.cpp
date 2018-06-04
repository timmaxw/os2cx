#include "poly_map_internal_masks.hpp"

namespace os2cx {

bool nef_contains(const os2cx::CgalNef3 &nef, const CGAL::Point_3<KE> &p) {
    os2cx::CgalNef3::Object_handle obj = nef.locate(p);
    os2cx::CgalNef3::Vertex_const_handle v_h;
    if (assign(v_h, obj)) return v_h->mark();
    os2cx::CgalNef3::Halfedge_const_handle he_h;
    if (assign(he_h, obj)) return he_h->mark();
    os2cx::CgalNef3::Halffacet_const_handle hf_h;
    if (assign(hf_h, obj)) return hf_h->mark();
    os2cx::CgalNef3::Volume_const_handle vo_h;
    if (assign(vo_h, obj)) return vo_h->mark();
    assert(false);
}

/* Given a Nef polyhedron volume, three points on a triangulation on one of its
facets, and the plane of the facet, returns a point on the interior of the
volume. */
CGAL::Point_3<KE> calculate_interior_point(
    const os2cx::CgalNef3 &nef,
    os2cx::CgalNef3::Volume_const_handle volume,
    const CGAL::Point_3<KE> &p0,
    const CGAL::Point_3<KE> &p1,
    const CGAL::Point_3<KE> &p2
) {
    CGAL::Vector_3<KE> v01(p0, p1);
    CGAL::Vector_3<KE> v02(p0, p2);

    /* This is just an arbitrary point on the interior of the triangle */
    CGAL::Point_3<KE> point_on_triangle = p0 + v01/3 + v02/3;

    /* This is an arbitrary vector pointing from the triangle in towards the
    volume */
    CGAL::Vector_3<KE> normal = CGAL::cross_product(v01, v02);

    /* We start at a point on the interior of the triangle and step a
    distance 'offset' towards the interior of the volume. However, if the
    volume is very thin there's a risk we will overshoot. So if we don't
    end up in the target volume then we divide 'offset' by 10 and try
    again. */
    KE::FT offset = 1;
    int tries = 100;
    for (int i = 0; i < tries; ++i) {
        CGAL::Point_3<KE> guess = point_on_triangle + normal * offset;
        os2cx::CgalNef3::Object_handle handle = nef.locate(guess);
        os2cx::CgalNef3::Volume_const_handle volume2;
        if (assign(volume2, handle) && volume == volume2) {
            return guess;
        }
        offset /= 10;
    }

    /* This probably indicates a bug, but it could conceivably happen if there
    were a volume with a thickness less than 10^-100 */
    assert(false);
}

void maybe_compute_volume_masks(
    const Poly3Map &poly3_map,
    Poly3Map::VolumeId volume_id,
    Poly3Map::VertexId p0,
    Poly3Map::VertexId p1,
    Poly3Map::VertexId p2,
    const std::vector<os2cx::CgalNef3> &mask_nefs,
    const std::vector<std::set<Poly3Map::VolumeId> *> &mask_volumes_out
) {
    if (!poly3_map.volumes[volume_id].is_solid) {
        // no need to compute masks for this one
        return;
    }

    CGAL::Point_3<KE> interior_point = calculate_interior_point(
        poly3_map.i->nef,
        poly3_map.i->volume_to_nef[volume_id],
        poly3_map.i->vertex_to_nef[p0]->point(),
        poly3_map.i->vertex_to_nef[p1]->point(),
        poly3_map.i->vertex_to_nef[p2]->point());

    for (int i = 0; i < static_cast<int>(mask_nefs.size()); ++i) {
        const os2cx::CgalNef3 &mask_nef = mask_nefs[i];
        if (nef_contains(mask_nef, interior_point)) {
            mask_volumes_out.at(i)->insert(volume_id);
        }
    }
}

void poly3_map_internal_compute_volume_mask_volumes(
    const Poly3Map &poly3_map,
    const std::vector<os2cx::CgalNef3> &volume_mask_nefs,
    const std::vector<std::set<Poly3Map::VolumeId> *>
        &volume_mask_volumes_out
) {
    assert(volume_mask_nefs.size() == volume_mask_volumes_out.size());
    for (const Poly3Map::Surface &surface : poly3_map.surfaces) {
        maybe_compute_volume_masks(
            poly3_map,
            surface.volumes[0],
            surface.triangles[0].vertices[0],
            surface.triangles[0].vertices[1],
            surface.triangles[0].vertices[2],
            volume_mask_nefs,
            volume_mask_volumes_out);
        maybe_compute_volume_masks(
            poly3_map,
            surface.volumes[1],
            surface.triangles[0].vertices[0],
            surface.triangles[0].vertices[2],
            surface.triangles[0].vertices[1],
            volume_mask_nefs,
            volume_mask_volumes_out);
    }
}

} /* namespace os2cx */
