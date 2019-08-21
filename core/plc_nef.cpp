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

PlcNef3 PlcNef3::empty() {
    PlcNef3 res;
    res.i.reset(new PlcNef3Internal(CgalNef3Plc::EMPTY));
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
        ci->mark() = PlcNef3Mark(func(ci->mark().attrs));
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

        AttrBitset
            facet_attrs = hfi->mark().attrs,
            volume1_attrs = hfi->incident_volume()->mark().attrs,
            volume2_attrs = hfi->twin()->incident_volume()->mark().attrs;
        CGAL::Vector_3<KE> cgal_normal = hfi->plane().orthogonal_vector();
        Vector normal = Vector(
            CGAL::to_double(cgal_normal.x()),
            CGAL::to_double(cgal_normal.y()),
            CGAL::to_double(cgal_normal.z()));
        normal /= normal.magnitude();

        PlcNef3Mark new_mark(func(
            facet_attrs,
            volume1_attrs,
            volume2_attrs,
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
            PlcNef3Mark new_mark(func(svi->mark().attrs));
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
        vi->mark() = PlcNef3Mark(func(vi->mark().attrs));
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

void PlcNef3::map_volumes(const std::function<AttrBitset(AttrBitset)> &func) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_volumes(sncd, func);
    });
}

void PlcNef3::map_faces(
    const std::function<AttrBitset(
        AttrBitset current_attrs,
        AttrBitset volume1_attrs,
        AttrBitset volume2_attrs,
        Vector normal_towards_volume1
    )> &func
) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_faces(sncd, func);
    });
}

void PlcNef3::map_edges(const std::function<AttrBitset(AttrBitset)> &func) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_edges(sncd, func);
    });
}

void PlcNef3::map_vertices(const std::function<AttrBitset(AttrBitset)> &func) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_vertices(sncd, func);
    });
}

void PlcNef3::map_everywhere(
    const std::function<AttrBitset(AttrBitset, FeatureType)> &func
) {
    plc_nef_3_modify(this, [&](CgalNef3Sncd *sncd) {
        plc_nef_3_modify_volumes(sncd, [&](AttrBitset attrs) {
            return func(attrs, FeatureType::Volume);
        });
        plc_nef_3_modify_faces(sncd,
        [&](AttrBitset attrs, AttrBitset, AttrBitset, Vector) {
            return func(attrs, FeatureType::Face);
        });
        plc_nef_3_modify_edges(sncd, [&](AttrBitset attrs) {
            return func(attrs, FeatureType::Edge);
        });
        plc_nef_3_modify_vertices(sncd, [&](AttrBitset attrs) {
            return func(attrs, FeatureType::Vertex);
        });
    });
}

void PlcNef3::binarize(AttrBitset attrs_one, AttrBitset attrs_zero)
{
    map_everywhere([&](AttrBitset prev, FeatureType) {
        if (prev != AttrBitset()) {
            return attrs_one;
        } else {
            return attrs_zero;
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
                    shei->source()->mark().attrs | shei->mark().attrs);
                vi->mark() = PlcNef3Mark(
                    vi->mark().attrs | shei->mark().attrs);
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

AttrBitset PlcNef3::get_attrs(Point point) const {
    CGAL::Point_3<KE> point2(point.x, point.y, point.z);
    CgalNef3Plc::Object_handle obj = i->p.locate(point2);

    CgalNef3Plc::Vertex_const_handle vh;
    if (CGAL::assign(vh, obj)) return vh->mark().attrs;
    CgalNef3Plc::Halfedge_const_handle eh;
    if (CGAL::assign(eh, obj)) return eh->mark().attrs;
    CgalNef3Plc::Halffacet_const_handle fh;
    if (CGAL::assign(fh, obj)) return fh->mark().attrs;
    CgalNef3Plc::Volume_const_handle ch;
    if (CGAL::assign(ch, obj)) return ch->mark().attrs;

    assert(false);
}

} /* namespace os2cx */
