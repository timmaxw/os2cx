#include "plc_nef_to_plc.hpp"

#include "plc_nef.internal.hpp"

namespace os2cx {

/* triangulate_nef_facet() and PlcTriangulationHandler were copied with
modifications from CGAL/Nef_polyhedron_3.h */

template<typename Kernel>
class PlcTriangulationHandler {
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
        CTVertex_handle, CgalNef3Plc::Vertex_const_handle
        > ctv2v;
    KE::Plane_3 supporting_plane;

public:
    PlcTriangulationHandler(
            CgalNef3Plc::Halffacet_const_handle f) :
        visited(false), supporting_plane(f->plane())
    {
        CgalNef3Plc::Halffacet_cycle_const_iterator fci;
        for (fci=f->facet_cycles_begin(); fci!=f->facet_cycles_end(); ++fci) {
            if (fci.is_shalfedge()) {
                CgalNef3Plc::SHalfedge_around_facet_const_circulator
                    sfc(fci), send(sfc);
                CGAL_For_all(sfc,send) {
                    CgalNef3Plc::Vertex_const_handle v =
                        sfc->source()->source();
                    CTVertex_handle ctv = ct.insert(v->point());
                    ctv2v[ctv] = v;
                }
            } else {
                CgalNef3Plc::SHalfloop_const_handle shl = fci;
                CgalNef3Plc::Vertex_const_handle v =
                    shl->incident_sface()->center_vertex();
                CTVertex_handle ctv = ct.insert(v->point());
                ctv2v[ctv] = v;
            }
        }

        for (fci=f->facet_cycles_begin();
                fci!=f->facet_cycles_end(); ++fci) {
            if (fci.is_shalfedge()) {
                CgalNef3Plc::SHalfedge_around_facet_const_circulator
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
            CgalNef3Plc::Vertex_const_handle vs[3] = {
                ctv2v[fi->vertex(0)],
                ctv2v[fi->vertex(1)],
                ctv2v[fi->vertex(2)],
            };
            CGAL::Plane_3<KE> plane(
                vs[0]->point(), vs[1]->point(), vs[2]->point());
            if (!same_orientation(plane)) {
                std::swap(vs[1], vs[2]);
            }
            callback(vs);
        }
    }
};

template<class Callable>
void triangulate_nef_facet(
    CgalNef3Plc::Halffacet_const_handle f,
    const Callable &callback
) {
    CgalNef3Plc::SHalfedge_const_handle se;
    CgalNef3Plc::Halffacet_cycle_const_iterator fc;

    CgalNef3Plc::SHalfedge_around_facet_const_circulator
      sfc1(f->facet_cycles_begin()), sfc2(sfc1);

    if (++f->facet_cycles_begin() == f->facet_cycles_end() &&
            ++(++(++sfc1)) == sfc2) {
        /* The facet is a triangle. This is a very common case, so we handle it
        with a specialized fast path */
        fc = f->facet_cycles_begin();
        se = CgalNef3Plc::SHalfedge_const_handle(fc);
        CGAL_assertion(se!=0);
        CgalNef3Plc::SHalfedge_around_facet_const_circulator hc(se);
        CgalNef3Plc::Vertex_const_handle vs[3];
        vs[0] = hc->source()->center_vertex();
        ++hc;
        vs[1] = hc->source()->center_vertex();
        ++hc;
        vs[2] = hc->source()->center_vertex();
        callback(vs);
    } else {
        /* The facet is not just a triangle, and it could in principle be quite
        complex with concavities, interior holes, etc. Fully triangulate it. */
        CGAL::Vector_3<KE> orth = f->plane().orthogonal_vector();
        int c = CGAL::abs(orth[0]) > CGAL::abs(orth[1]) ? 0 : 1;
        c = CGAL::abs(orth[2]) > CGAL::abs(orth[c]) ? 2 : c;

        if (c == 0) {
            PlcTriangulationHandler<CGAL::Projection_traits_yz_3<KE> > th(f);
            th.handle_triangles(callback);
        } else if (c == 1) {
            PlcTriangulationHandler<CGAL::Projection_traits_xz_3<KE> > th(f);
            th.handle_triangles(callback);
        } else if (c == 2) {
            PlcTriangulationHandler<CGAL::Projection_traits_xy_3<KE> > th(f);
            th.handle_triangles(callback);
        } else {
            CGAL_error_msg( "wrong value");
        }
    }
}

class PlcConverter {
public:
    PlcConverter(const PlcNef3 &plc_nef) :
        nef(plc_nef.i->p),
        vertex_index(nef.vertices_begin(), nef.vertices_end()),
        volume_index(nef.volumes_begin(), nef.volumes_end()),
        halffacet_index(nef.halffacets_begin(), nef.halffacets_end())
        { }

    void make_vertices() {
        plc.vertices.reserve(nef.number_of_vertices());
        CgalNef3Plc::Vertex_const_iterator vi;
        CGAL_forall_vertices(vi, nef) {
            assert(vertex_index[vi] == plc.vertices.size());
            Plc3::Vertex vertex;
            vertex.point = Point(
                CGAL::to_double(vi->point().x()),
                CGAL::to_double(vi->point().y()),
                CGAL::to_double(vi->point().z()));
            vertex.bitset = vi->mark().bitset;
            plc.vertices.push_back(vertex);
        }
    }

    void make_volumes() {
        plc.volumes.reserve(nef.number_of_volumes());
        CgalNef3Plc::Volume_const_iterator ci;
        CGAL_forall_volumes(ci, nef) {
            assert(volume_index[ci] == plc.volumes.size());
            Plc3::Volume volume;
            volume.bitset = ci->mark().bitset;
            plc.volumes.push_back(volume);
        }

        /* Construct a test point that has a smaller x-coordinate than of any of
        the vertices of the PlcNef3, so it's guaranteed to lie in the outside
        volume */
        KE::FT min_x;
        CgalNef3Plc::Vertex_const_iterator vi;
        CGAL_forall_vertices(vi, nef) {
            min_x = std::min(min_x, vi->point().x());
        }
        CGAL::Point_3<KE> test_point(min_x - 1, 0, 0);
        CgalNef3Plc::Object_handle obj = nef.locate(test_point);
        CgalNef3Plc::Volume_const_handle volume_outside;
        bool ok = assign(volume_outside, obj);
        assert(ok);
        plc.volume_outside = volume_index[volume_outside];
    }

    /* is_border_halfedge() checks if there's something 'special' about the
    given halfedge, such that it should be part of a Border instead of embedded
    into a Surface. */
    bool is_border_halfedge(CgalNef3Plc::Halfedge_const_handle he) {
        assert(!he->is_isolated());
        CgalNef3Plc::SHalfedge_const_handle sedge_1, sedge_2;
        sedge_1 = he->out_sedge();
        if (sedge_1->mark() != he->mark()) {
            /* The edge is differently marked than the adjacent face, so it must
            be a border */
            return true;
        }
        sedge_2 = sedge_1->cyclic_adj_succ();
        if (sedge_2 == sedge_1) {
            /* The edge is adjacent to only one face; the surface terminates in
            the middle of a volume. */
            return true;
        }
        if (sedge_2->mark() != he->mark()) {
            /* The edge is differently marked than the other adjacent face, so
            it must be a border. */
            return true;
        }
        if (sedge_1 != sedge_2->cyclic_adj_succ()) {
            /* The edge is adjacent to at least three faces */
            return true;
        }
        /* The edge is adjacent to exactly two faces, and both faces and the
        edge have exactly the same marks, so it's not a border halfedge. */
        return false;
    }

    void make_surfaces() {
        halffacet_surfaces = std::vector<Plc3::SurfaceId>(
            nef.number_of_halffacets(), -1);
        halffacet_orientations = std::vector<bool>(
            nef.number_of_halffacets());

        CgalNef3Plc::Halffacet_const_iterator seed;
        CGAL_forall_halffacets(seed, nef) {
            /* If we already processed this halffacet as part of a surface
            seeded from some other halffacet, don't repeat it */
            if (halffacet_surfaces[halffacet_index[seed]] != -1) continue;

            Plc3::VolumeId vol0 = volume_index[seed->incident_volume()];
            Plc3::VolumeId vol1 = volume_index[seed->twin()->incident_volume()];

            /* For any given pair of halffacets, we only want to process one of
            them; so we skip the one incident on the larger-numbered volume. If
            they are incident on the same volume, we'll process one side first
            at random and then naturally skip the other. because
            halffacet_surfaces will have been filled. */
            if (vol0 > vol1) continue;

            Plc3::SurfaceId surface_id = plc.surfaces.size();
            Plc3::Surface surface;
            surface.volumes[0] = vol0;
            surface.volumes[1] = vol1;
            surface.bitset = seed->mark().bitset;

            /* Breadth-first search to find all the facets that should be part
            of this surface */
            std::deque<CgalNef3Plc::Halffacet_const_handle> queue;
            queue.push_back(seed);
            while (!queue.empty()) {
                CgalNef3Plc::Halffacet_const_handle h = queue.front();
                queue.pop_front();

                /* Double-check that we didn't accidentally cross onto a facet
                that should be part of a different surface */
                assert(h->incident_volume() == seed->incident_volume());
                assert(h->twin()->incident_volume() ==
                    seed->twin()->incident_volume());
                assert(h->mark().bitset == seed->mark().bitset);

                /* Check whether we already visited this facet */
                int index = halffacet_index[h];
                if (halffacet_surfaces[index] != -1) {
                    /* We already visited this one via a different route */
                    assert(halffacet_surfaces[index] == surface_id);
                    assert(halffacet_orientations[index] == true);
                    continue;
                }

                /* Mark this facet as visited */
                halffacet_surfaces[index] = surface_id;
                halffacet_orientations[index] = true;

                /* Mark this facet's twin as visited */
                int twin_index = halffacet_index[h->twin()];
                assert(halffacet_surfaces[twin_index] == -1);
                halffacet_surfaces[twin_index] = surface_id;
                halffacet_orientations[twin_index] = false;

                /* Generate triangles for this facet */
                triangulate_nef_facet(h,
                [&](CgalNef3Plc::Vertex_const_handle *vs) {
                    Plc3::Surface::Triangle tri;
                    for (int i = 0; i < 3; ++i) {
                        tri.vertices[i] = vertex_index[vs[i]];
                    }
                    surface.triangles.push_back(tri);
                });

                /* Push neighboring facets onto the queue if they should be part
                of the same surface */
                for (auto fci = h->facet_cycles_begin();
                        fci != h->facet_cycles_end(); ++fci) {
                    if (!fci.is_shalfedge()) {
                        continue;
                    }
                    CgalNef3Plc::SHalfedge_const_handle she = fci;
                    do {
                        CgalNef3Plc::Halfedge_const_handle he = she->target();
                        if (is_border_halfedge(he)) {
                            continue;
                        }
                        CgalNef3Plc::Halffacet_const_handle next_h =
                            she->snext()->facet();
                        if (halffacet_surfaces[halffacet_index[next_h]] == -1) {
                            queue.push_back(next_h);
                        }
                    } while ((she = she->next()) != fci);
                }
            }

            plc.surfaces.push_back(std::move(surface));
        }
    }

    void make_borders() {
        std::set<std::pair<Plc3::VertexId, Plc3::VertexId> > todo;
        std::map<Plc3::VertexId, int> vertex_counts;

        /* Collect all the border halfedges and store them by vertex IDs */
        CgalNef3Plc::Halfedge_const_iterator hi;
        CGAL_forall_halfedges(hi, nef) {
            if (!is_border_halfedge(hi)) {
                continue;
            }
            Plc3::VertexId v0 = vertex_index[hi->source()];
            Plc3::VertexId v1 = vertex_index[hi->target()];
            todo.insert(std::make_pair(v0, v1));
            ++vertex_counts[v0];
        }

        /* Collect contiguous sequences of border halfedges into plc.borders */
        CGAL_forall_halfedges(hi, nef) {
            if (!is_border_halfedge(hi)) {
                continue;
            }
            Plc3::VertexId v0 = vertex_index[hi->source()];
            Plc3::VertexId v1 = vertex_index[hi->target()];
            auto todo_it = todo.find(std::make_pair(v0, v1));
            if (todo_it == todo.end()) {
                /* This halfedge is already part of a border */
                continue;
            }

            Plc3::Border border;
            border.bitset = hi->mark().bitset;

            /* Calculate the surfaces incident to this border */
            CgalNef3Plc::SHalfedge_const_handle
                she = hi->out_sedge(), she_end = she;
            do {
                Plc3::SurfaceId surface =
                    halffacet_surfaces[halffacet_index[she->facet()]];
                border.surfaces.push_back(surface);
            } while ((she = she->cyclic_adj_succ()) != she_end);

            todo.erase(todo_it);
            todo.erase(std::make_pair(v1, v0));

            /* Follow a contiguous sequence of border halfedges in both
            directions from this starting point */
            for (int direction = 1; direction <= 2; ++direction) {
                Plc3::VertexId cur = (direction == 1) ? v1 : v0;
                Plc3::VertexId prev = (direction == 1) ? v0 : v1;
                while (true) {
                    assert(todo.count(std::make_pair(cur, prev)) == 0);
                    assert(todo.count(std::make_pair(prev, cur)) == 0);

                    if (direction == 1) border.vertices.push_front(cur);
                    else border.vertices.push_back(cur);

                    if (vertex_counts[cur] != 2 ||
                            plc.vertices[cur].bitset != border.bitset) {
                        /* End the sequence if we found a dead end, a multi-way
                        junction, or a vertex with different bits set */
                        break;
                    }

                    /* Find the next outgoing link from here */
                    auto next_it = todo.lower_bound(std::make_pair(cur, 0));
                    if (next_it == todo.end() || next_it->first != cur) {
                        /* The outgoing link from here was already removed from
                        'todo'. This should only happen if this border is a
                        loop. */
                        if (direction == 1) {
                            /* We came back around to the start. */
                            assert(cur == v0 && prev != v1);
                        } else {
                            /* We haven't gone anywhere because the very first
                            link was already removed from 'todo' during the
                            'direction == 1' phase. */
                            assert(cur == v0 && prev == v1);
                            assert(border.vertices.back() == v0);
                        }
                        break;
                    }

                    prev = cur;
                    cur = next_it->second;
                    todo.erase(next_it);
                    todo.erase(std::make_pair(cur, prev));
                }
            }

            plc.borders.push_back(border);
        }
    }

    const CgalNef3Plc &nef;
    Plc3 plc;

    CGAL::Inverse_index<CgalNef3Plc::Vertex_const_handle> vertex_index;
    CGAL::Inverse_index<CgalNef3Plc::Volume_const_handle> volume_index;
    CGAL::Inverse_index<CgalNef3Plc::Halffacet_const_handle> halffacet_index;
    std::vector<Plc3::SurfaceId> halffacet_surfaces;
    std::vector<bool> halffacet_orientations;
};

Plc3 plc_nef_to_plc(const PlcNef3 &plc_nef) {
    PlcConverter converter(plc_nef);
    converter.make_vertices();
    converter.make_volumes();
    converter.make_surfaces();
    converter.make_borders();
    return std::move(converter.plc);
}

} /* namespace os2cx */
