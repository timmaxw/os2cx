#include "poly_map.internal.hpp"

#include <CGAL/Nef_nary_union_3.h>
#include <CGAL/point_generators_3.h>

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
        for (const auto &pair : volumes[vid].masks) {
            stream << " " << pair.first << "=";
            if (pair.second) stream << "Y";
            else stream << "N";
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

/* Assuming the given Nef polyhedron is bounded, it must have a unique outside
volume; volume_outside() finds this volume and returns it. */
os2cx::CgalNef3::Volume_const_handle volume_outside(
    const os2cx::CgalNef3 &nef
) {
    KE::FT min_x;
    for (auto it = nef.vertices_begin(); it != nef.vertices_end(); ++it) {
        min_x = std::min(min_x, it->point().x());
    }
    CGAL::Point_3<KE> test_point(min_x-1, 0, 0);
    os2cx::CgalNef3::Object_handle obj = nef.locate(test_point);
    os2cx::CgalNef3::Volume_const_handle volume;
    bool ok = assign(volume, obj);
    assert(ok);
    return volume;
}

bool is_border_halfedge(os2cx::CgalNef3::Halfedge_const_handle he) {
    assert(!he->is_isolated());
    return he->out_sedge()->cyclic_adj_succ()->cyclic_adj_succ()
        != he->out_sedge();
}

class BorderTraversal {
private:
    typedef std::set<std::pair<
        Poly3Map::VertexId,
        Poly3Map::VertexId
        > > LinkSet;

public:
    BorderTraversal(Poly3Map *pmo) :
        poly3_map_out(pmo)
        { }

    void run() {
        for (auto it = poly3_map_out->i->nef.halfedges_begin();
                it != poly3_map_out->i->nef.halfedges_end(); ++it) {
            if (!is_border_halfedge(it)) continue;
            Poly3Map::VertexId source =
                poly3_map_out->i->vertex_from_nef[it->source()];
            Poly3Map::VertexId target =
                poly3_map_out->i->vertex_from_nef[it->target()];
            links.insert(std::make_pair(source, target));
        }

        while (!links.empty()) {
            Poly3Map::BorderId border_id =
                poly3_map_out->borders.size();
            poly3_map_out->borders.push_back(
                Poly3Map::Border());
            Poly3Map::Border *border =
                &poly3_map_out->borders.back();

            Poly3Map::VertexId v1 = links.begin()->first;
            Poly3Map::VertexId v2 = links.begin()->second;
            note_used_link(links.begin(), border_id);

            border->vertices.push_back(v1);
            border->vertices.push_back(v2);
            follow_chain(border, border_id, v2, v1, true);
            follow_chain(border, border_id, v1, v2, false);
        }
    }

private:
    void note_used_link(
        LinkSet::iterator it,
        Poly3Map::BorderId border_id
    ) {
        Poly3Map::VertexId v1 = it->first, v2 = it->second;
        links.erase(it);
        links.erase(std::make_pair(v2, v1));
        poly3_map_out->borders_by_vertex.insert(std::make_pair(
            std::make_pair(std::min(v1, v2), std::max(v1, v2)),
            border_id));
    }

    void follow_chain(
        Poly3Map::Border *border,
        Poly3Map::BorderId border_id,
        Poly3Map::VertexId previous,
        Poly3Map::VertexId current,
        bool front
    ) {
        while (true) {
            LinkSet::iterator next_it;
            int count = 0;
            for (auto it = links.lower_bound(std::make_pair(current, 0));
                    it != links.lower_bound(std::make_pair(current + 1, 0));
                    ++it) {
                assert(it->first == current);
                if (it->second != previous) {
                    ++count;
                    next_it = it;
                }
            }
            if (count != 1) return;
            if (front) {
                border->vertices.push_front(next_it->second);
            } else {
                border->vertices.push_back(next_it->second);
            }
            current = next_it->second;
            previous = current;
            note_used_link(next_it, border_id);
        }
    }

    Poly3Map *poly3_map_out;
    LinkSet links;
};

/* triangulate_nef_facet() and CGAL_RMapTriangulationHandler2 were copied with
modifications from CGAL/Nef_polyhedron_3.h */

template<typename Kernel>
class CGAL_RMapTriangulationHandler2 {
    typedef typename CGAL::Triangulation_vertex_base_2<Kernel> Vb;
    typedef typename CGAL::Constrained_triangulation_face_base_2<Kernel> Fb;
    typedef typename CGAL::Triangulation_data_structure_2<Vb,Fb> TDS;
    typedef typename CGAL::Constrained_triangulation_2<Kernel,TDS> CT;

    typedef typename CT::Face_handle           Face_handle;
    typedef typename CT::Vertex_handle         CTVertex_handle;
    typedef typename CT::Finite_faces_iterator Finite_face_iterator;
    typedef typename CT::Edge                  Edge;

    CT ct;
    CGAL::Unique_hash_map<Face_handle, bool> visited;
    CGAL::Unique_hash_map<
        CTVertex_handle, os2cx::CgalNef3::Vertex_const_handle
        > ctv2v;
    KE::Plane_3 supporting_plane;

public:
    CGAL_RMapTriangulationHandler2(
            os2cx::CgalNef3::Halffacet_const_handle f) : 
        visited(false), supporting_plane(f->plane())
    {
        os2cx::CgalNef3::Halffacet_cycle_const_iterator fci;
        for (fci=f->facet_cycles_begin(); fci!=f->facet_cycles_end(); ++fci) {
            if (fci.is_shalfedge()) {
                os2cx::CgalNef3::SHalfedge_around_facet_const_circulator
                    sfc(fci), send(sfc);
                CGAL_For_all(sfc,send) {
                    CTVertex_handle ctv =
                        ct.insert(sfc->source()->source()->point());
                    ctv2v[ctv] = sfc->source()->source();
                }
            }
        }

        for (fci=f->facet_cycles_begin();
                fci!=f->facet_cycles_end(); ++fci) {
            if (fci.is_shalfedge()) {
                os2cx::CgalNef3::SHalfedge_around_facet_const_circulator
                    sfc(fci), send(sfc);
                CGAL_For_all(sfc,send) {
                    ct.insert_constraint(
                        sfc->source()->source()->point(),
                        sfc->source()->twin()->source()->point());
                }
            }
        }
        CGAL_assertion(ct.is_valid());

        typename CT::Face_handle infinite = ct.infinite_face();
        typename CT::Vertex_handle ctv = infinite->vertex(1);
        if(ct.is_infinite(ctv)) ctv = infinite->vertex(2);
        CGAL_assertion(!ct.is_infinite(ctv));

        typename CT::Face_handle opposite;
        typename CT::Face_circulator vc(ctv,infinite);
        do { opposite = vc++;
        } while (!ct.is_constrained(
            typename CT::Edge(vc,vc->index(opposite))));
        typename CT::Face_handle first = vc;

        CGAL_assertion(!ct.is_infinite(first));
        traverse_triangulation(first, first->index(opposite));
    }

    void traverse_triangulation(Face_handle f, int parent) {
        visited[f] = true;
        if (!ct.is_constrained(Edge(f,ct.cw(parent))) &&
                !visited[f->neighbor(ct.cw(parent))]) {
            Face_handle child(f->neighbor(ct.cw(parent)));
            traverse_triangulation(child, child->index(f));
        } 
        if (!ct.is_constrained(Edge(f,ct.ccw(parent))) &&
                !visited[f->neighbor(ct.ccw(parent))]) {
            Face_handle child(f->neighbor(ct.ccw(parent)));
            traverse_triangulation(child, child->index(f));
        } 
    } 

    bool same_orientation(KE::Plane_3 p1) const {
        if(p1.a() != 0)
            return CGAL::sign(p1.a()) == CGAL::sign(supporting_plane.a());
        if(p1.b() != 0)
            return CGAL::sign(p1.b()) == CGAL::sign(supporting_plane.b());
        return CGAL::sign(p1.c()) == CGAL::sign(supporting_plane.c());
    }

    template<class Callable>
    void handle_triangles(const Callable &callback) {
        for (Finite_face_iterator fi = ct.finite_faces_begin();
                fi != ct.finite_faces_end(); ++fi) {
            if (visited[fi] == false) continue;
            KE::Plane_3 plane(
                fi->vertex(0)->point(),
                fi->vertex(1)->point(),
                fi->vertex(2)->point());
            if (same_orientation(plane)) {
                callback(
                    ctv2v[fi->vertex(0)],
                    ctv2v[fi->vertex(1)],
                    ctv2v[fi->vertex(2)]);
            } else {
                callback(
                    ctv2v[fi->vertex(0)],
                    ctv2v[fi->vertex(2)],
                    ctv2v[fi->vertex(1)]);
            }
        }
    }
};

template<class Callable>
void triangulate_nef_facet(
    os2cx::CgalNef3::Halffacet_const_handle f,
    const Callable &callback
) {
    os2cx::CgalNef3::SHalfedge_const_handle se;
    os2cx::CgalNef3::Halffacet_cycle_const_iterator fc;

    os2cx::CgalNef3::SHalfedge_around_facet_const_circulator 
      sfc1(f->facet_cycles_begin()), sfc2(sfc1);
    
    if (++f->facet_cycles_begin() != f->facet_cycles_end() ||
            ++(++(++sfc1)) != sfc2) {
        CGAL::Vector_3<KE> orth = f->plane().orthogonal_vector();
        int c = CGAL::abs(orth[0]) > CGAL::abs(orth[1]) ? 0 : 1;
        c = CGAL::abs(orth[2]) > CGAL::abs(orth[c]) ? 2 : c;
      
        if (c == 0) {
            os2cx::CGAL_RMapTriangulationHandler2<
                CGAL::Projection_traits_yz_3<KE> > th(f);
            th.handle_triangles(callback);
        } else if (c == 1) {
            os2cx::CGAL_RMapTriangulationHandler2<
                CGAL::Projection_traits_xz_3<KE> > th(f);
            th.handle_triangles(callback);
        } else if (c == 2) {
            os2cx::CGAL_RMapTriangulationHandler2<
                CGAL::Projection_traits_xy_3<KE> > th(f);
            th.handle_triangles(callback);
        } else {
            CGAL_error_msg( "wrong value");
        }

    } else {
        fc = f->facet_cycles_begin();
        se = os2cx::CgalNef3::SHalfedge_const_handle(fc);
        CGAL_assertion(se!=0);
        os2cx::CgalNef3::SHalfedge_around_facet_const_circulator hc(se);
        os2cx::CgalNef3::Vertex_const_handle v1, v2, v3;
        v1 = hc->source()->center_vertex();
        ++hc;
        v2 = hc->source()->center_vertex();
        ++hc;
        v3 = hc->source()->center_vertex();
        callback(v1, v2, v3);
    }
}

class SurfaceTraversal {
public:
    SurfaceTraversal(Poly3Map *rmo) :
        poly3_map_out(rmo),
        nef(poly3_map_out->i->nef),
        facet_ids(nef.halffacets_begin(), nef.halffacets_end()),
        facets_visited(nef.number_of_halffacets(), false)
        { }

    void run() {
        for (auto seed = nef.halffacets_begin();
                seed != nef.halffacets_end(); ++seed) {
            /* If we already processed this halffacet as part of a surface
            seeded from an earlier halffacet, then don't process it again */
            if (facets_visited[facet_ids[seed]]) continue;

            Poly3Map::VolumeId volume0 =
                poly3_map_out->i->volume_from_nef[
                    seed->incident_volume()];
            Poly3Map::VolumeId volume1 =
                poly3_map_out->i->volume_from_nef[
                    seed->twin()->incident_volume()];

            /* For any given pair of halffacets, we only want to process one of
            them; so we skip the one incident on the larger-numbered volume. */
            if (volume0 > volume1) continue;

            Poly3Map::SurfaceId surface_id =
                poly3_map_out->surfaces.size();
            poly3_map_out->surfaces.push_back(
                Poly3Map::Surface());
            Poly3Map::Surface *surface =
                &poly3_map_out->surfaces.back();
            surface->volumes[0] = volume0;
            surface->volumes[1] = volume1;

            /* Perform a breadth-first search starting from *it for all
            halffacets on the same surface. */
            std::deque<os2cx::CgalNef3::Halffacet_const_handle> queue;
            queue.push_back(seed);
            while (!queue.empty()) {
                os2cx::CgalNef3::Halffacet_const_handle next = queue.front();
                queue.pop_front();
                if (facets_visited[facet_ids[next]]) continue;
                facets_visited[facet_ids[next]] = true;

                assert(next->incident_volume() ==
                    seed->incident_volume());
                assert(next->twin()->incident_volume() ==
                    seed->twin()->incident_volume());

                triangulate(next, surface);

                spread_to_neighbors(next, &queue, surface_id);
            }
        }
    }

    void triangulate(
        os2cx::CgalNef3::Halffacet_const_handle hf,
        Poly3Map::Surface *surface
    ) {
        CGAL::Vector_3<K> normal = CGAL::Cartesian_converter<KE, K>()(
            hf->plane().orthogonal_vector());
        normal /= sqrt(
            normal.x() * normal.x() +
            normal.y() * normal.y() +
            normal.z() * normal.z());

        triangulate_nef_facet(hf, [&](
            os2cx::CgalNef3::Vertex_const_handle v0,
            os2cx::CgalNef3::Vertex_const_handle v1,
            os2cx::CgalNef3::Vertex_const_handle v2
        ) {
            surface->triangles.push_back(
                Poly3Map::Surface::Triangle());
            auto &triangle = surface->triangles.back();
            triangle.vertices[0] = poly3_map_out->i->vertex_from_nef[v0];
            triangle.vertices[1] = poly3_map_out->i->vertex_from_nef[v1];
            triangle.vertices[2] = poly3_map_out->i->vertex_from_nef[v2];
            triangle.normal = PureVector(
                PureScalar(normal.x()),
                PureScalar(normal.y()),
                PureScalar(normal.z()));
        });
    }

    void spread_to_neighbors(
        os2cx::CgalNef3::Halffacet_const_handle hf,
        std::deque<os2cx::CgalNef3::Halffacet_const_handle> *queue,
        Poly3Map::SurfaceId surface_id
    ) {
        for (auto fc_it = hf->facet_cycles_begin();
                fc_it != hf->facet_cycles_end(); ++fc_it) {
            assert(fc_it.is_shalfedge());
            os2cx::CgalNef3::SHalfedge_const_handle
                sh_start = fc_it, sh = sh_start;
            for (bool done = false; !done;
                    sh = sh->next(), done = (sh == sh_start)) {
                os2cx::CgalNef3::Halfedge_const_handle he = sh->target();
                if (is_border_halfedge(he)) {
                    Poly3Map::VertexId v1 =
                        poly3_map_out->i->vertex_from_nef[he->source()];
                    Poly3Map::VertexId v2 =
                        poly3_map_out->i->vertex_from_nef[he->target()];
                    Poly3Map::BorderId border_id =
                        poly3_map_out->borders_by_vertex[
                            std::make_pair(std::min(v1, v2), std::max(v1, v2))];
                    poly3_map_out->borders[border_id].surfaces.insert(
                        surface_id);
                } else {
                    os2cx::CgalNef3::Halffacet_const_handle other_hf =
                        sh->snext()->facet();
                    assert(other_hf->incident_volume() ==
                        hf->incident_volume());
                    assert(other_hf->twin()->incident_volume() ==
                        hf->twin()->incident_volume());
                    if (!facets_visited[facet_ids[other_hf]]) {
                        queue->push_back(other_hf);
                    }
                }
            }
        }
    }

private:
    Poly3Map *poly3_map_out;
    const os2cx::CgalNef3 &nef;
    CGAL::Inverse_index<os2cx::CgalNef3::Halffacet_const_handle> facet_ids;
    std::vector<bool> facets_visited;
};

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
    Poly3Map *poly3_map_out,
    const std::vector<const Poly3 *> &masks,
    const std::vector<os2cx::CgalNef3> &mask_nefs,
    Poly3Map::VolumeId volume_id,
    Poly3Map::VertexId p0,
    Poly3Map::VertexId p1,
    Poly3Map::VertexId p2
) {
    Poly3Map::Volume *volume =
        &poly3_map_out->volumes[volume_id];
    if (!volume->is_solid) return;   // no need to compute masks for this one
    if (masks.empty()) return;   // nothing to do
    if (!volume->masks.empty()) return;   // we already did this one

    CGAL::Point_3<KE> interior_point = calculate_interior_point(
        poly3_map_out->i->nef,
        poly3_map_out->i->volume_to_nef[volume_id],
        poly3_map_out->i->vertex_to_nef[p0]->point(),
        poly3_map_out->i->vertex_to_nef[p1]->point(),
        poly3_map_out->i->vertex_to_nef[p2]->point());

    for (int i = 0; i < static_cast<int>(masks.size()); ++i) {
        const Poly3 *mask = masks[i];
        const os2cx::CgalNef3 &mask_nef = mask_nefs[i];
        volume->masks[mask] = nef_contains(mask_nef, interior_point);
    }
}

void poly3_map_create(
    const Poly3 &solid,
    const std::vector<const Poly3 *> &masks,
    Poly3Map *poly3_map_out
) {
    std::vector<os2cx::CgalNef3> mask_nefs;
    for (const Poly3 *mask : masks) {
        mask_nefs.push_back(poly3_nef(*mask));
    }

    poly3_map_out->i.reset(new Poly3MapInternal);
    if (!masks.empty()) {
        /* Set 'poly3_map_out->i->nef' to 'solid' minus the union of the mask
        boundaries. So each volume of 'nef' will belong to a different region of
        the poly3 map. */
        CGAL::Nef_nary_union_3<os2cx::CgalNef3> mask_union;
        for (const os2cx::CgalNef3 &mask_nef : mask_nefs) {
            mask_union.add_polyhedron(mask_nef.boundary());
        }
        poly3_map_out->i->nef =
            poly3_nef(solid).difference(mask_union.get_union());
    } else {
        poly3_map_out->i->nef = poly3_nef(solid);
    }

    const os2cx::CgalNef3 &nef = poly3_map_out->i->nef;

    /* Populate 'vertices', 'vertex_from_nef', and 'vertex_to_nef' */
    poly3_map_out->vertices.reserve(nef.number_of_vertices());
    poly3_map_out->i->vertex_to_nef.reserve(nef.number_of_vertices());
    for (auto it = nef.vertices_begin(); it != nef.vertices_end(); ++it) {
        poly3_map_out->i->vertex_from_nef.push_back(it);
        poly3_map_out->i->vertex_to_nef.push_back(it);
        Poly3Map::Vertex v;
        v.point = Point::raw(
            CGAL::to_double(it->point().x()),
            CGAL::to_double(it->point().y()),
            CGAL::to_double(it->point().z()));
        poly3_map_out->vertices.push_back(std::move(v));
    }

    /* Populate 'volumes', 'volume_from_nef' and 'volume_to_nef', but don't
    fill 'Volume::masks' yet. */
    poly3_map_out->volumes.reserve(nef.number_of_volumes());
    poly3_map_out->volume_outside = -1;
    for (auto it = nef.volumes_begin(); it != nef.volumes_end(); ++it) {
        poly3_map_out->i->volume_from_nef.push_back(it);
        poly3_map_out->i->volume_to_nef.push_back(it);
        if (!it->mark()) {
            assert(poly3_map_out->volume_outside == -1);
            poly3_map_out->volume_outside =
                poly3_map_out->volumes.size();
        }
        poly3_map_out->volumes.push_back(
            Poly3Map::Volume());
        poly3_map_out->volumes.back().is_solid = it->mark();
    }

    /* Populate 'volume_outside' */
    poly3_map_out->volume_outside =
        poly3_map_out->i->volume_from_nef[volume_outside(nef)];

    /* Populate 'borders' and 'borders_by_vertex', but don't fill
    'Border::surfaces' yet. */
    BorderTraversal(poly3_map_out).run();

    /* Populate 'surfaces' and fill 'Border::surfaces' */
    SurfaceTraversal(poly3_map_out).run();

    /* Fill 'Volume::masks' (we couldn't do this before surface traversal
    because we need triangulation data) */
    if (!masks.empty()) {
        for (const Poly3Map::Surface &surface :
                poly3_map_out->surfaces) {
            maybe_compute_volume_masks(
                poly3_map_out,
                masks,
                mask_nefs,
                surface.volumes[0],
                surface.triangles[0].vertices[0],
                surface.triangles[0].vertices[1],
                surface.triangles[0].vertices[2]);
            maybe_compute_volume_masks(
                poly3_map_out,
                masks,
                mask_nefs,
                surface.volumes[1],
                surface.triangles[0].vertices[0],
                surface.triangles[0].vertices[2],
                surface.triangles[0].vertices[1]);
        }
    }
}

} /* namespace os2cx */


