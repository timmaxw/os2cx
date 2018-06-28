#ifndef OS2CX_PLC_NEF_HPP_
#define OS2CX_PLC_NEF_HPP_

#include <bitset>
#include <functional>
#include <memory>

#include "calc.hpp"
#include "plc.hpp"
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
    typedef Plc3::Bitset Bitset;
    enum class FeatureType { Volume, Face, Edge, Vertex };

    PlcNef3();
    PlcNef3(PlcNef3 &&other);
    ~PlcNef3();
    PlcNef3 &operator=(PlcNef3 &&other);
    PlcNef3 clone() const;

    /* Returns a PlcNef3 for which the surface and interior of the given
    polyhedron are set to all-ones, and the exterior is set to all-zeroes. */
    static PlcNef3 from_poly(const Poly3 &poly);

    /* Applies the given function to the bits of every volume, in place */
    void map_volumes(const std::function<Bitset(Bitset)> &func);

    /* Applies the given function to the bits of every face, in place. */
    void map_faces(
        const std::function<Bitset(
            Bitset current_bitset,
            Bitset volume1_bitset,
            Bitset volume2_bitset,
            Vector normal_towards_volume1
        )> &func);

    /* Applies the given function to the bits of every edge, in place */
    void map_edges(const std::function<Bitset(Bitset)> &func);

    /* Applies the given function to the bits of every vertex, in place */
    void map_vertices(const std::function<Bitset(Bitset)> &func);

    /* Applies the given function to the bits of every feature, in place. */
    void map_everywhere(const std::function<Bitset(Bitset, FeatureType)> &func);

    /* Replaces every non-zero bitset with bitset_one, and every all-zeroes
    bitset with bitset_zero. */
    void binarize(Bitset bitset_one, Bitset bitset_zero);

    /* For each edge of each face, do 'edge.bitset |= face.bitset', and the same
    with each vertex of each face. */
    void outline_faces();

    /* Returns Poly3Bits formed by performing the given binary operations on
    this and other. */
    PlcNef3 binary_or(const PlcNef3 &other) const;
    PlcNef3 binary_and(const PlcNef3 &other) const;
    PlcNef3 binary_and_not(const PlcNef3 &other) const;
    PlcNef3 binary_xor(const PlcNef3 &other) const;

    Bitset get_bitset(Point point) const;

    std::unique_ptr<PlcNef3Internal> i;
};

} /* namespace os2cx */

#endif /* OS2CX_PLC_NEF_HPP_ */
