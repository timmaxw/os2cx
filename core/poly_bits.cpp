#include "poly_bits.internal.hpp"

namespace os2cx {

Poly3Bits::Poly3Bits() { }
Poly3Bits::Poly3Bits(Poly3Bits &&other) : i(std::move(other.i)) { }
Poly3Bits::~Poly3Bits() { }
Poly3Bits &Poly3Bits::operator=(Poly3Bits &&other) {
    i = std::move(other.i);
    return *this;
}

Poly3Bits Poly3Bits::from_poly(const Poly3 &poly) {
    CGAL::Polyhedron_3<KE> poly_exact;
    convert_polyhedron(poly.i->p, &poly_exact);
    Poly3Bits poly_bits;
    poly_bits.i = std::make_unique<Poly3BitsInternal>(poly_exact);
    return poly_bits;
}

template<class Callable>
class Poly3BitsModifier :
    public CGAL::Modifier_base<CgalNef3Bits::SNC_structure>
{
public:
    Poly3BitsModifier(const Callable &f) : func(f) { }
    void operator()(SNC_structure &snc) const {
        CgalNef3Bits::SNC_decorator snc_decorator(snc);

        Vertex_iterator v;
        CGAL_forall_vertices(v, snc_decorator) {
            v->mark() = func(v->mark(), Poly3Bits::FeatureType::Vertex);

            CgalNef3Bits::SM_decorator sm_decorator(&*v);

            CgalNef3Bits::Sphere_map::SVertex_iterator sv;
            CGAL_forall_svertices(sv, sm_decorator) {
                sv->mark() = func(sv->mark(), Poly3Bits::FeatureType::Edge);
            }

            CgalNef3Bits::Sphere_map::SHalfedge_iterator se;
            CGAL_forall_shalfedges(se, sm_decorator) {
                se->mark() = func(se->mark(), Poly3Bits::FeatureType::Face);
            }

            if (sm_decorator.has_shalfloop()) {
                CgalNef3Bits::Sphere_map::SHalfloop_handle shl = shalfloop();
                shl->mark() = shl->twin()->mark() =
                    func(shl->mark(), Poly3Bits::FeatureType::Face);
            }

            CgalNef3Bits::Sphere_map::SFace_iterator sf;
            CGAL_forall_sfaces(sf, sm_decorator) {
                sf->mark() = func(se->mark(), Poly3Bits::FeatureType::Volume);
            }
        }

        Halffacet_iterator f;
        CGAL_forall_halffacets(f, snc_decorator) {
            f->mark() = func(f->mark(), Poly3Bits::FeatureType::Face);
        }

        Volume_iterator c;
        CGAL_forall_volumes(c, snc_decorator) {
            c->mark() = func(c->mark(), Poly3Bits::FeatureType::Volume);
        }
    }
    const Callable &func;
};

void Poly3Bits::everywhere_map(
    const std::function<Bitset(Bitset, FeatureType)> &func
) {
    Poly3BitsModifier modifier(callable);
    i->p.delegate(modifier);
}

Poly3Bits Poly3Bits::binary_or(const Poly3Bits &other) const {
    Poly3Bits res;
    res.i = std::make_unique<Poly3BitsInternal>(
        i->p.join(other.i->p));
    return res;
}

Poly3Bits Poly3Bits::binary_and(const Poly3Bits &other) const {
    Poly3Bits res;
    res.i = std::make_unique<Poly3BitsInternal>(
        i->p.intersection(other.i->p));
    return res;
}

Poly3Bits Poly3Bits::binary_and_not(const Poly3Bits &other) const {
    Poly3Bits res;
    res.i = std::make_unique<Poly3BitsInternal>(
        i->p.difference(other.i->p));
    return res;
}

Poly3Bits Poly3Bits::binary_xor(const Poly3Bits &other) const {
    Poly3Bits res;
    res.i = std::make_unique<Poly3BitsInternal>(
        i->p.symmetric_difference(other.i->p));
    return res;
}

Poly3Bits::Bitset Poly3Bits::get_bitset(Point point) const {
    CGAL::Point_3<KE> point2(
        point.vector.x.val,
        point.vector.y.val,
        point.vector.z.val);
    Poly3BitsNef::Object_handle obj = i->p.locate(point2);

    Poly3BitsNef::Vertex_const_handle vh;
    if (CGAL::assign(vh, obj)) return vh->mark();
    Poly3BitsNef::Halfedge_const_handle eh;
    if (CGAL::assign(eh, obj)) return eh->mark();
    Poly3BitsNef::Halffacet_const_handle fh;
    if (CGAL::assign(fh, obj)) return fh->mark();
    Poly3BitsNef::Volume_const_handle vh;
    if (CGAL::assign(vh, obj)) return vh->mark();

    assert(false);
}

} /* namespace os2cx */
