#ifndef OS2CX_PLC_NEF_HPP_
#define OS2CX_PLC_NEF_HPP_

#include <bitset>
#include <functional>
#include <memory>

#include "calc.hpp"
#include "poly.hpp"

namespace os2cx {

/* PlcNef3 is a piecewise-linear complex where each volume, surface, edge, and
vertex is annotated with a bitset. It supports arbitrary boolean operations that
combine overlapping PlcNef3s and alter their bitsets. */

/* PlcNef3 is internally implemented as a CGAL::Nef_polyhedron_3 with a custom
Marks type, but CGAL headers take a long time to compile, so the implementation
is hidden in PlcNef3Internal. */
class PlcNef3Internal;

class PlcNef3
{
public:
    static const int num_bits = 64;
    typedef std::bitset<num_bits> Bitset;

    enum class FeatureType { Volume, Face, Edge, Vertex };

    PlcNef3();
    PlcNef3(PlcNef3 &&other);
    ~PlcNef3();
    PlcNef3 &operator=(PlcNef3 &&other);
    PlcNef3 clone() const;

    /* Returns a Poly3Bits for which the surface and interior of the given
    polyhedron are set to all-ones, and the exterior is set to all-zeroes. */
    static PlcNef3 from_poly(const Poly3 &poly);

    /* Applies the given function to the bits of every feature, in place. */
    void everywhere_map(
        const std::function<Bitset(Bitset, FeatureType)> &func);

    /* Replaces all bitsets except for all-zeroes with the given bitset. */
    void everywhere_binarize(Bitset value);

    /* Returns Poly3Bits formed by performing the given binary operations on
    *this and other. */
    PlcNef3 binary_or(const PlcNef3 &other) const;
    PlcNef3 binary_and(const PlcNef3 &other) const;
    PlcNef3 binary_and_not(const PlcNef3 &other) const;
    PlcNef3 binary_xor(const PlcNef3 &other) const;

    Bitset get_bitset(Point point) const;

    std::unique_ptr<PlcNef3Internal> i;
};

} /* namespace os2cx */

#endif /* OS2CX_PLC_NEF_HPP_ */
