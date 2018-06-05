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

template<class Callable>
void plc_nef_3_modify(PlcNef3 *plc, const Callable &func) {
    class PlcNef3Modifier :
        public CGAL::Modifier_base<CgalNef3Plc::Nef_rep::SNC_structure>
    {
    public:
        PlcNef3Modifier(const Callable &f) : func(f) { }
        void operator()(CgalNef3Plc::Nef_rep::SNC_structure &snc) {
            CgalNef3Plc::Nef_rep::SNC_decorator snc_decorator(snc);

            CgalNef3Plc::Nef_rep::SNC_decorator::Vertex_iterator v;
            CGAL_forall_vertices(v, snc_decorator) {
                v->mark() = PlcNef3Mark(
                    func(v->mark().bitset, PlcNef3::FeatureType::Vertex));

                CgalNef3Plc::SM_decorator sm_decorator(&*v);

                CgalNef3Plc::Nef_rep::Sphere_map::SVertex_iterator sv;
                CGAL_forall_svertices(sv, sm_decorator) {
                    sv->mark() = PlcNef3Mark(
                        func(sv->mark().bitset, PlcNef3::FeatureType::Edge));
                }

                CgalNef3Plc::Nef_rep::Sphere_map::SHalfedge_iterator se;
                CGAL_forall_shalfedges(se, sm_decorator) {
                    se->mark() = PlcNef3Mark(
                        func(se->mark().bitset, PlcNef3::FeatureType::Face));
                }

                if (sm_decorator.has_shalfloop()) {
                    CgalNef3Plc::Nef_rep::Sphere_map::SHalfloop_handle shl =
                        sm_decorator.shalfloop();
                    shl->mark() = shl->twin()->mark() = PlcNef3Mark(
                        func(shl->mark().bitset, PlcNef3::FeatureType::Face));
                }

                CgalNef3Plc::Nef_rep::Sphere_map::SFace_iterator sf;
                CGAL_forall_sfaces(sf, sm_decorator) {
                    sf->mark() = PlcNef3Mark(
                        func(sf->mark().bitset, PlcNef3::FeatureType::Volume));
                }
            }

            CgalNef3Plc::Nef_rep::SNC_decorator::Halffacet_iterator f;
            CGAL_forall_halffacets(f, snc_decorator) {
                f->mark() = PlcNef3Mark(
                    func(f->mark().bitset, PlcNef3::FeatureType::Face));
            }

            CgalNef3Plc::Nef_rep::SNC_decorator::Volume_iterator c;
            CGAL_forall_volumes(c, snc_decorator) {
                c->mark() = PlcNef3Mark(
                    func(c->mark().bitset, PlcNef3::FeatureType::Volume));
            }
        }
        const Callable &func;
    };
    PlcNef3Modifier modifier(func);
    plc->i->p.delegate(modifier);
}

void PlcNef3::everywhere_map(
    const std::function<Bitset(Bitset, FeatureType)> &func
) {
    plc_nef_3_modify(this, func);
}

void PlcNef3::everywhere_binarize(Bitset value) {
    plc_nef_3_modify(this, [&](Bitset prev, FeatureType) {
        if (prev == Bitset()) {
            return Bitset();
        } else {
            return value;
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
    CGAL::Point_3<KE> point2(
        point.vector.x.val,
        point.vector.y.val,
        point.vector.z.val);
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
