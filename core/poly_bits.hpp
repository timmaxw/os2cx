#ifndef OS2CX_POLY_BITS_HPP_
#define OS2CX_POLY_BITS_HPP_

#include <bitset>

#include "calc.hpp"

namespace os2cx {

/* Poly3Bits is a polyhedral cell complex where each volume, surface, edge, and
vertex is annotated with a bitset. It supports arbitrary boolean operations that
combine overlapping Poly3Bits and alter their bitsets. */

/* Poly3Bits is internally implemented as a CGAL::Nef_polyhedron_3 with a custom
Marks type, but CGAL headers take a long time to compile, so the implementation
is hidden in Poly3BitsInternal. */
class Poly3BitsInternal;

class Poly3Bits
{
public:
    static const int num_bits = 64;
    typedef std::bitset<num_bits> Bitset;

    enum class FeatureType { Volume, Face, Edge, Vertex };

    Poly3Bits();
    Poly3Bits(Poly3Bits &&other);
    ~Poly3Bits();
    Poly3Bits &operator=(Poly3Bits &&other);

    /* Returns a Poly3Bits for which the surface and interior of the given
    polyhedron have all bits set. */
    static Poly3Bits from_poly(const Poly3 &poly);

    /* Applies the given function to the bits of every feature, in place. */
    void everywhere_map(
        const std::function<Bitset(Bitset, FeatureType)> &func);

    /* Returns Poly3Bits formed by performing the given binary operations on
    *this and other. */
    Poly3Bits binary_or(const Poly3Bits &other) const;
    Poly3Bits binary_and(const Poly3Bits &other) const;
    Poly3Bits binary_and_not(const Poly3Bits &other) const;
    Poly3Bits binary_xor(const Poly3Bits &other) const;

    Bitset get_bitset(Point point) const;

    std::unique_ptr<Poly3BitsInternal> i;
};

} /* namespace os2cx */

#endif /* OS2CX_POLY_BITS_HPP_ */
