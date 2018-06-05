#ifndef OS2CX_POLY_BITS_INTERNAL_HPP_
#define OS2CX_POLY_BITS_INTERNAL_HPP_

#include "poly_bits.hpp"

#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

namespace os2cx {

/* Poly3BitsMarks is the custom Marks class for CGAL::Nef_polyhedron_3. It has
to imitate 'bool' in a bunch of awkward ways in order to be compatible. */
class Poly3BitsMarks {
public:
    Poly3BitsMarks() { }

    Poly3BitsMarks(bool b) {
        if (b) bitset.set();
        else bitset.reset();
    }

    explicit Poly3BitsMarks(Poly3Bits::Bitset s) : bitset(s) { }

    bool operator==(Poly3BitsMarks other) const {
        return bitset == other.bitset;
    }
    bool operator!=(Poly3BitsMarks other) const {
        return bitset != other.bitset;
    }

    Poly3BitsMarks operator!() const {
        return Poly3BitsMarks(~bitset);
    }
    Poly3BitsMarks operator&&(Poly3BitsMarks other) const {
        return Poly3BitsMarks(bitset & other.bitset);
    }
    Poly3BitsMarks operator||(Poly3BitsMarks other) const {
        return Poly3BitsMarks(bitset | other.bitset);
    }

    Poly3Bits::Bitset bitset;
}

inline ostream &operator<<(ostream &stream, Poly3BitMarks marks) {
    stream << '[';
    for (int i = 0; i < Poly3Bits::num_bits; ++i) {
        bool is_first = true;
        if (marks.bitset[i]) {
            if (is_first) is_first = false;
            else stream << ' ';
            stream << i;
        }
    }
    stream << ']';
    return stream;
}

typedef CGAL::Exact_predicates_exact_constructions_kernel KE;
typedef CGAL::Nef_polyhedron_3<
    KE,
    CGAL::Default_items<KE>::Items,
    Poly3BitsMarks
    > CgalNef3Bits;

class Poly3BitsInternal {
public:
    template<typename... Args>
    Poly3Internal(Args&&... args) : p(args...) { }
    os2cx::CgalNef3Bits p;
}

} /* namespace os2cx */

#endif /* OS2CX_POLY_BITS_INTERNAL_HPP_ */
