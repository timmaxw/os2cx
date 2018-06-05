#include "poly_map_internal_select.hpp"

namespace os2cx {

void poly3_map_internal_compute_cut_for_select_surface(
    const os2cx::CgalNef3 &solid_nef,
    const Poly3MapSelectSurface &select_surface,
    const os2cx::CgalNef3 &select_surface_nef,
    CGAL::Nef_nary_union_3<os2cx::CgalNef3> *cut_union_out
) {
    /* 'surface_candidate' will have marked faces for each part of the solid's
    surface that lies inside 'select_surface.poly'. So the ultimate selected
    surface will be a subset of those marked faces.

    There are a few subtleties here:
      * 'surface_candidate' also has marked volumes adjacent to the marked faces,
        on the side that was facing inwards towards the solid. This is so we can
        keep track of which way the face's normal was facing.
      * Those marked volumes have additional faces that weren't part of the
        surface of the original solid; those faces are unmarked.
    */
    os2cx::CgalNef3 surface_candidate =
        solid_nef.intersection(select_surface_nef.interior());

    for (auto it = surface_candidate.halfedges_begin();
            it != surface_candidate.halfedges_end(); ++it) {
        os2cx::CgalNef3::Halfedge_const_handle he = *it;

        /* Iterating over halfedges will visit each edge twice, once in each
        direction; but we only want to visit once, so arbitrarily discard one of
        the two halfedges. */
        os2cx::CgalNef3::Point_3 p1 = he->source()->point();
        os2cx::CgalNef3::Point_3 p2 = he->target()->point();
        if (p1 < p2) {
            continue;
        }

        if (he->is_isolated()) {
            /* This only happens in degenerate cases */
            continue;
        }

        /* The cut will be formed from every line segment that is adjacent to at
        least one selected face and at least one unselected face. */
        bool adjacent_selected_face = false;
        bool adjacent_unselected_face = false;

        /* Iterate over all the faces adjacent to this edge, stopping when we
        loop back around to the first one. */
        os2cx::CgalNef3::SHalfedge_const_handle first_sedge = he->out_sedge();
        bool is_first_iteration = true;
        for (os2cx::CgalNef3::SHalfedge_const_handle sedge = first_sedge;
                sedge != first_sedge || is_first_iteration;
                sedge = sedge->cyclic_adj_succ()) {
            is_first_iteration = false;

            os2cx::CgalNef3::Halffacet_const_handle facet = sedge->facet();
            if (!facet->mark()) {
                /* This facet is not a part of the solid's surface; rather, it's
                part of the surface of 'select_surface.poly' that was inside the
                solid's interior. The fact that this facet is adjacent to this
                edge implies that this edge is right on the edge of
                'select_surface.poly', so there was an adjacent facet of the
                solid's surface that was unselected because of
                'select_surface.poly'. */
                adjacent_unselected_face = true;
                continue;
            }

            os2cx::CgalNef3::Halffacet_const_handle facet2 = facet->twin();
            if (!facet->incident_volume()->mark() &&
                    facet2->incident_volume->mark()) {
                /* 'facet' is facing out from the solid and 'facet2' is facing
                in -- this is the way we want them */
            } else if (facet->incident_volume()->mark() &&
                    !facet2->incident_volume()->mark()) {
                /* 'facet' is the one facing into the solid; switch them so we
                can always assume that 'facet' is facing out */
                std::swap(facet, facet2);
            } else {
                /* Neither or both are facing into the solid. This should only
                happen in degenerate cases. */
                continue;
            }

            /* OK, this facet is on the surface of the solid and facing out, so
            it's selected if and only if it meets the 'select_surface.normal'
            criterion. */
            CGAL::Direction_3<os2cx::KE> normal =
                facet->plane()->orthogonal_direction();
            double dot =
                  normal.dx() * select_surface.normal_ref.x.val
                + normal.dy() * select_surface.normal_ref.y.val
                + normal.dz() * select_surface.normal_ref.z.val;
            bool selected = dot >= select_surface.normal_thresh;

            if (selected) {
                adjacent_selected_face = true;
            } else {
                adjacent_selected_face = false;
            }
        }

        if (adjacent_selected_face && adjacent_unselected_face) {
            /* This edge divides a selected face from an unselected face, so we
            should cut it out from 'poly_map->i->nef' in order to ensure that
            the selected face and the unselected face end up in different
            surfaces of the Poly3Map */
            CGAL::Segment_3<os2cx::KE> segment(p1, p2);
            cut_union_out->add_polyhedron(os2cx::CgalNef3(segment))
        }
    }
}

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

void maybe_compute_selected_volumes(
    const Poly3Map &poly3_map,
    Poly3Map::VolumeId volume_id,
    Poly3Map::VertexId p0,
    Poly3Map::VertexId p1,
    Poly3Map::VertexId p2,
    const std::vector<os2cx::CgalNef3> &select_volume_nefs,
    const std::vector<std::set<Poly3Map::VolumeId> *> &selected_volumes_out
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

    for (int i = 0; i < static_cast<int>(select_volume_nefs.size()); ++i) {
        const os2cx::CgalNef3 &select_nef = select_volume_nefs[i];
        if (nef_contains(mask_nef, interior_point)) {
            selected_volumes_out.at(i)->insert(volume_id);
        }
    }
}

void poly3_map_internal_compute_selected_volumes(
    const Poly3Map &poly3_map,
    const std::vector<os2cx::CgalNef3> &select_volume_nefs,
    const std::vector<std::set<Poly3Map::VolumeId> *>
        &selected_volumes_out
) {
    assert(select_volume_nefs.size() == selected_volumes_out.size());
    for (const Poly3Map::Surface &surface : poly3_map.surfaces) {
        maybe_compute_selected_volumes(
            poly3_map,
            surface.volumes[0],
            surface.triangles[0].vertices[0],
            surface.triangles[0].vertices[1],
            surface.triangles[0].vertices[2],
            select_volume_nefs,
            selected_volumes_out);
        maybe_compute_selected_volumes(
            poly3_map,
            surface.volumes[1],
            surface.triangles[0].vertices[0],
            surface.triangles[0].vertices[2],
            surface.triangles[0].vertices[1],
            select_volume_nefs,
            selected_volumes_out);
    }
}

void poly3_map_internal_compute_selected_surfaces(
    const Poly3Map &poly3_map,
    const std::vector<Poly3MapSelectSurface> &select_surfaces,
    const std::vector<os2cx::CgalNef3> &select_surface_nefs,
    const std::vector<std::set<Poly3Map::SurfaceId> *>
        &selected_surfaces_out
) {
    assert(select_surfaces.size() == select_surface_nefs.size());
    assert(selected_surfaces.size() == selected_surfaces_out.size());
    for (Poly3Map::SurfaceId surface_id = 0;
            surface_id < static_cast<int>(poly3_map.surfaces.size());
            ++surface_id) {

    }
}

} /* namespace os2cx */
