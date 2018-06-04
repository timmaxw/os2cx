#include "poly_map.internal.hpp"

#include <CGAL/Nef_nary_union_3.h>
#include <CGAL/point_generators_3.h>

#include "poly_map_internal_traversal.hpp"
#include "poly_map_internal_masks.hpp"

namespace os2cx {

Poly3Map::Poly3Map() { }
Poly3Map::~Poly3Map() { }

DebugNef::DebugNef(const os2cx::CgalNef3 &n) :
    nef(n),
    vertex_index(nef.vertices_begin(), nef.vertices_end()),
    halfedge_index(nef.halfedges_begin(), nef.halfedges_end()),
    halffacet_index(nef.halffacets_begin(), nef.halffacets_end()),
    volume_index(nef.volumes_begin(), nef.volumes_end())
    { }

void DebugNef::dump(std::ostream &stream) const {
    stream << "NEF " << std::endl;
    if (nef.is_simple()) {
        stream << "simple";
    } else {
        stream << "unsimple";
    }
    stream << " vertices=" << nef.number_of_vertices()
        << " halfedges=" << nef.number_of_halfedges()
        << " edges=" << nef.number_of_edges()
        << " halffacets=" << nef.number_of_halffacets()
        << " facets=" << nef.number_of_facets()
        << " volumes=" << nef.number_of_volumes()
        << std::endl;

    CGAL::Inverse_index<typename os2cx::CgalNef3::Vertex_const_iterator>
        vertex_index(nef.vertices_begin(), nef.vertices_end());
    CGAL::Inverse_index<typename os2cx::CgalNef3::Halfedge_const_iterator>
        halfedge_index(nef.halfedges_begin(), nef.halfedges_end());
    CGAL::Inverse_index<typename os2cx::CgalNef3::Halffacet_const_iterator>
        halffacet_index(nef.halffacets_begin(), nef.halffacets_end());
    CGAL::Inverse_index<typename os2cx::CgalNef3::Volume_const_iterator>
        volume_index(nef.volumes_begin(), nef.volumes_end());

    stream << "VERTICES:" << std::endl;
    for (auto it = nef.vertices_begin(); it != nef.vertices_end(); ++it) {
        stream << "V" << vertex_index[it];
        if (it->mark()) stream << " mark";
        stream << " point=" << it->point()
            << std::endl;
    }

    stream << "HALFEDGES:" << std::endl;
    for (auto it = nef.halfedges_begin(); it != nef.halfedges_end(); ++it) {
        stream << "HE" << halfedge_index[it];
        if (it->mark()) stream << " mark";
        if (it->is_isolated()) stream << " isolated";
        stream << " source=V" << vertex_index[it->source()];
        stream << " target=V" << vertex_index[it->target()];
        stream << " twin=HE" << halfedge_index[it->twin()];
        stream << std::endl;
    }

    stream << "HALFFACETS:" << std::endl;
    for (auto it = nef.halffacets_begin(); it != nef.halffacets_end(); ++it) {
        stream << "HF" << halffacet_index[it];
        if (it->mark()) stream << " mark";
        stream << " twin=HF" << halffacet_index[it->twin()];
        stream << " volume=VO" << volume_index[it->incident_volume()];
        for (auto jt = it->facet_cycles_begin();
                jt != it->facet_cycles_end(); ++jt) {
            stream << " edges";
        }
        stream << std::endl;
    }

    stream << "VOLUMES:" << std::endl;
    for (auto it = nef.volumes_begin(); it != nef.volumes_end(); ++it) {
        stream << "VO" << volume_index[it];
        if (it->mark()) stream << " mark";
        for (auto jt = it->shells_begin(); jt != it->shells_end(); ++jt) {
            stream << " shell";
        }
        stream << std::endl;
    }
}

void Poly3Map::debug(std::ostream &stream) const {
    stream << "VERTICES " << vertices.size() << std::endl;
    for (Poly3Map::VertexId vid = 0;
            vid < static_cast<int>(vertices.size()); ++vid) {
        stream << "V" << vid << ' ' << vertices[vid].point << std::endl;
    }
    stream << "VOLUMES " << volumes.size() << std::endl;
    for (Poly3Map::VolumeId vid = 0;
            vid < static_cast<int>(volumes.size()); ++vid) {
        stream << "VO" << vid;
        if (vid == volume_outside) {
            stream << " outside";
        }
        stream << std::endl;
    }
    stream << "SURFACES " << surfaces.size() << std::endl;
    for (Poly3Map::SurfaceId sid = 0;
            sid < static_cast<int>(surfaces.size()); ++sid) {
        stream << "S" << sid << " VO" << surfaces[sid].volumes[0]
            << " VO" << surfaces[sid].volumes[1];
        for (const Poly3Map::Surface::Triangle &triangle
                : surfaces[sid].triangles) {
            stream << " V" << triangle.vertices[0]
                << "-V" << triangle.vertices[1]
                << "-V" << triangle.vertices[2];
        }
        stream << std::endl;
    }
    stream << "BORDERS " << borders.size() << std::endl;
    for (Poly3Map::BorderId bid = 0;
            bid < static_cast<int>(borders.size()); ++bid) {
        stream << "B" << bid;
        for (Poly3Map::VertexId vid : borders[bid].vertices) {
            stream << ' ' << vertices[vid].point;
        }
        for (Poly3Map::SurfaceId sid : borders[bid].surfaces) {
            stream << " S" << sid;
        }
        stream << std::endl;
    }
}

os2cx::CgalNef3 poly3_nef(const Poly3 &poly3) {
    CGAL::Polyhedron_3<KE> poly_exact;
    convert_polyhedron(poly3.i->p, &poly_exact);
    return os2cx::CgalNef3(poly_exact);
}

void poly3_map_create(
    const Poly3 &solid,
    const std::vector<Poly3MapVolumeMask> &volume_masks,
    Poly3Map *poly3_map_out,
    const std::vector<std::set<Poly3Map::VolumeId> *>
        &volume_mask_volumes_out
) {
    std::vector<os2cx::CgalNef3> volume_mask_nefs;
    for (const Poly3MapVolumeMask &mask : volume_masks) {
        volume_mask_nefs.push_back(poly3_nef(*mask.poly));
    }

    poly3_map_out->i.reset(new Poly3MapInternal);
    if (!volume_masks.empty()) {
        /* Set 'poly3_map_out->i->nef' to 'solid' minus the union of the mask
        boundaries. So each volume of 'nef' will belong to a different region of
        the poly3 map. */
        CGAL::Nef_nary_union_3<os2cx::CgalNef3> mask_union;
        for (const os2cx::CgalNef3 &mask_nef : volume_mask_nefs) {
            mask_union.add_polyhedron(mask_nef.boundary());
        }
        poly3_map_out->i->nef =
            poly3_nef(solid).difference(mask_union.get_union());
    } else {
        poly3_map_out->i->nef = poly3_nef(solid);
    }

    /* Fill publicly visible fields of Poly3Map */
    poly3_map_internal_traverse_vertices(poly3_map_out);
    poly3_map_internal_traverse_volumes(poly3_map_out);
    poly3_map_internal_traverse_borders(poly3_map_out);
    poly3_map_internal_traverse_surfaces(poly3_map_out);

    /* Fill 'volume_mask_volumes_out' (we couldn't do this before surface
    traversal because we need triangulation data) */
    poly3_map_internal_compute_volume_mask_volumes(
        *poly3_map_out, volume_mask_nefs, volume_mask_volumes_out);
}

} /* namespace os2cx */


