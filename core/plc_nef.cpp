#include "plc_nef.internal.hpp"

#include "poly.internal.hpp"

namespace os2cx {

PlcNef3::PlcNef3() { }
PlcNef3::PlcNef3(PlcNef3 &&other) : i(std::move(other.i)) { }
PlcNef3::~PlcNef3() { }
PlcNef3 &PlcNef3::operator=(PlcNef3 &&other) {
    i = std::move(other.i);
    return *this;
}
PlcNef3 PlcNef3::clone() const {
    PlcNef3 res;
    res.i.reset(new PlcNef3Internal(i->p));
    return res;
}

PlcNef3 PlcNef3::from_poly(const Poly3 &poly) {
    /* TODO: this doesn't work for polyhedra with multiple parts or internal
    holes */
    CGAL::Polyhedron_3<KE> poly_exact;
    convert_polyhedron(poly.i->p, &poly_exact);
    PlcNef3 poly_bits;
    poly_bits.i.reset(new PlcNef3Internal(poly_exact));
    return poly_bits;
}

typedef CgalNef3Plc::Nef_rep::SNC_decorator CgalNef3Sncd;

template<class Callable>
void plc_nef_3_modify_volumes(
    CgalNef3Sncd *snc_decorator,
    const Callable &func
) {
    CgalNef3Sncd::Volume_iterator ci;
    CGAL_forall_volumes(ci, *snc_decorator) {
        ci->mark() = PlcNef3Mark(func(ci->mark().bitset));
    }

    /* Update sface marks to match new volume marks */
    CgalNef3Sncd::Vertex_iterator vi;
    CGAL_forall_vertices(vi, *snc_decorator) {
        CgalNef3Plc::SM_decorator sm_decorator(&*vi);

        CgalNef3Plc::Nef_rep::Sphere_map::SFace_iterator sfi;
        CGAL_forall_sfaces(sfi, sm_decorator) {
            sfi->mark() = sfi->volume()->mark();
        }
    }
}

template<class Callable>
void plc_nef_3_modify_faces(
    CgalNef3Sncd *snc_decorator,
    const Callable &func
) {
    CgalNef3Sncd::Halffacet_iterator hfi;
    CGAL_forall_halffacets(hfi, *snc_decorator) {
        /* Visit each facet only once, so skip one of the two halffacets */
        if (hfi < hfi->twin()) continue;

        Plc3::Bitset
            facet_bitset = hfi->mark().bitset,
            volume1_bitset = hfi->incident_volume()->mark().bitset,
            volume2_bitset = hfi->twin()->incident_volume()->mark().bitset;
        CGAL::Vector_3<KE> cgal_normal = hfi->plane().orthogonal_vector();
        Vector normal = Vector(
            CGAL::to_double(cgal_normal.x()),
            CGAL::to_double(cgal_normal.y()),
            CGAL::to_double(cgal_normal.z()));
        normal /= normal.magnitude();

        PlcNef3Mark new_mark(func(
            facet_bitset,
            volume1_bitset,
            volume2_bitset,
            normal
        ));

        hfi->mark() = new_mark;
        hfi->twin()->mark() = new_mark;
    }

    /* Update shalfedge/shalfloop marks to match new facet marks */
    CgalNef3Sncd::Vertex_iterator vi;
    CGAL_forall_vertices(vi, *snc_decorator) {
        CgalNef3Plc::SM_decorator sm_decorator(&*vi);

        CgalNef3Plc::Nef_rep::Sphere_map::SHalfedge_iterator shei;
        CGAL_forall_shalfedges(shei, sm_decorator) {
            shei->mark() = shei->facet()->mark();
        }

        if (sm_decorator.has_shalfloop()) {
            CgalNef3Plc::Nef_rep::Sphere_map::SHalfloop_handle shlh =
                sm_decorator.shalfloop();
            shlh->mark() = shlh->twin()->mark() = shlh->facet()->mark();
        }
    }
}

template<class Callable>
void plc_nef_3_modify_edges(
    CgalNef3Sncd *snc_decorator,
    const Callable &func
) {
    CgalNef3Plc::Nef_rep::SNC_decorator::Vertex_iterator vi;
    CGAL_forall_vertices(vi, *snc_decorator) {
        CgalNef3Plc::SM_decorator sm_decorator(&*vi);

        CgalNef3Plc::Nef_rep::Sphere_map::SVertex_iterator svi;
        CGAL_forall_svertices(svi, sm_decorator) {
            /* Visit each edge only once, so skip one of the two halfedges */
            if (svi < svi->twin()) continue;
            PlcNef3Mark new_mark(func(svi->mark().bitset));
            svi->mark() = svi->twin()->mark() = new_mark;
        }
    }
}

template<class Callable>
void plc_nef_3_modify_vertices(
    CgalNef3Sncd *snc_decorator,
    const Callable &func
) {
    CgalNef3Plc::Nef_rep::SNC_decorator::Vertex_iterator vi;
    CGAL_forall_vertices(vi, *snc_decorator) {
        vi->mark() = PlcNef3Mark(func(vi->mark().bitset));
    }
}

template<class Callable>
void plc_nef_3_modify(PlcNef3 *plc, const Callable &func) {
    class PlcNef3Modifier :
        public CGAL::Modifier_base<CgalNef3Plc::Nef_rep::SNC_structure>
    {
    public:
        PlcNef3Modifier(const Callable &f) : func(f) { }
        void operator()(CgalNef3Plc::Nef_rep::SNC_structure &snc) {
            CgalNef3Sncd snc_decorator(snc);
            func(&snc_decorator);
        }
        const Callable &func;
    };
    PlcNef3Modifier modifier(func);
    plc->i->p.delegate(modifier);
}

void PlcNef3::map_volumes(const std::function<Bitset(Bitset)> &func) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_volumes(sncd, func);
    });
}

void PlcNef3::map_faces(
    const std::function<Bitset(
        Bitset current_bitset,
        Bitset volume1_bitset,
        Bitset volume2_bitset,
        Vector normal_towards_volume1
    )> &func
) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_faces(sncd, func);
    });
}

void PlcNef3::map_edges(const std::function<Bitset(Bitset)> &func) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_edges(sncd, func);
    });
}

void PlcNef3::map_vertices(const std::function<Bitset(Bitset)> &func) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_vertices(sncd, func);
    });
}

void PlcNef3::map_everywhere(
    const std::function<Bitset(Bitset, FeatureType)> &func
) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_volumes(sncd, [&](Plc3::Bitset bitset) {
            return func(bitset, FeatureType::Volume);
        });
        plc_nef_3_modify_faces(sncd,
        [&](Plc3::Bitset bitset, Plc3::Bitset, Plc3::Bitset, Vector) {
            return func(bitset, FeatureType::Face);
        });
        plc_nef_3_modify_edges(sncd, [&](Plc3::Bitset bitset) {
            return func(bitset, FeatureType::Edge);
        });
        plc_nef_3_modify_vertices(sncd, [&](Plc3::Bitset bitset) {
            return func(bitset, FeatureType::Vertex);
        });
    });
}

void PlcNef3::binarize(PlcNef3::Bitset bitset_one, PlcNef3::Bitset bitset_zero)
{
    map_everywhere([&](Bitset prev, FeatureType) {
        if (prev != Bitset()) {
            return bitset_one;
        } else {
            return bitset_zero;
        }
    });
}

void PlcNef3::outline_faces() {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        CgalNef3Plc::Nef_rep::SNC_decorator::Vertex_iterator vi;
        CGAL_forall_vertices(vi, *sncd) {
            CgalNef3Plc::SM_decorator sm_decorator(&*vi);

            CgalNef3Plc::Nef_rep::Sphere_map::SHalfedge_iterator shei;
            CGAL_forall_shalfedges(shei, sm_decorator) {
                shei->source()->mark() = PlcNef3Mark(
                    shei->source()->mark().bitset | shei->mark().bitset);
                vi->mark() = PlcNef3Mark(
                    vi->mark().bitset | shei->mark().bitset);
            }
        }
    });
}

PlcNef3 PlcNef3::binary_or(const PlcNef3 &other) const {
    PlcNef3 res;
    res.i.reset(new PlcNef3Internal(i->p.join(other.i->p)));
    return res;
}

PlcNef3 PlcNef3::binary_and(const PlcNef3 &other) const {
    PlcNef3 res;
    res.i.reset(new PlcNef3Internal(i->p.intersection(other.i->p)));
    return res;
}

PlcNef3 PlcNef3::binary_and_not(const PlcNef3 &other) const {
    PlcNef3 res;
    res.i.reset(new PlcNef3Internal(i->p.difference(other.i->p)));
    return res;
}

PlcNef3 PlcNef3::binary_xor(const PlcNef3 &other) const {
    PlcNef3 res;
    res.i.reset(new PlcNef3Internal(i->p.symmetric_difference(other.i->p)));
    return res;
}

PlcNef3::Bitset PlcNef3::get_bitset(Point point) const {
    CGAL::Point_3<KE> point2(point.x, point.y, point.z);
    CgalNef3Plc::Object_handle obj = i->p.locate(point2);

    CgalNef3Plc::Vertex_const_handle vh;
    if (CGAL::assign(vh, obj)) return vh->mark().bitset;
    CgalNef3Plc::Halfedge_const_handle eh;
    if (CGAL::assign(eh, obj)) return eh->mark().bitset;
    CgalNef3Plc::Halffacet_const_handle fh;
    if (CGAL::assign(fh, obj)) return fh->mark().bitset;
    CgalNef3Plc::Volume_const_handle ch;
    if (CGAL::assign(ch, obj)) return ch->mark().bitset;

    assert(false);
}

} /* namespace os2cx */
