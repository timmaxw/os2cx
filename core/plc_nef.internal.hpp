#ifndef OS2CX_POLY_BITS_INTERNAL_HPP_
#define OS2CX_POLY_BITS_INTERNAL_HPP_

#include "plc_nef.hpp"

#include <ostream>

#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

namespace os2cx {

/* Poly3BitsMark is the custom Mark class for CGAL::Nef_polyhedron_3. It has
to imitate 'bool' in a bunch of awkward ways in order to be compatible. */
class PlcNef3Mark {
public:
    PlcNef3Mark() { }

    PlcNef3Mark(bool b) {
        if (b) attrs.set();
        else attrs.reset();
    }

    explicit PlcNef3Mark(AttrBitset s) : attrs(s) { }

    bool operator==(PlcNef3Mark other) const {
        return attrs == other.attrs;
    }
    bool operator!=(PlcNef3Mark other) const {
        return attrs != other.attrs;
    }

    PlcNef3Mark operator!() const {
        return PlcNef3Mark(~attrs);
    }
    PlcNef3Mark operator&&(PlcNef3Mark other) const {
        return PlcNef3Mark(attrs & other.attrs);
    }
    PlcNef3Mark operator||(PlcNef3Mark other) const {
        return PlcNef3Mark(attrs | other.attrs);
    }

    AttrBitset attrs;
};

inline std::ostream &operator<<(std::ostream &stream, PlcNef3Mark marks) {
    stream << '[';
    bool is_first = true;
    for (int i = 0; i < num_attr_bits; ++i) {
        if (marks.attrs[i]) {
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
    PlcNef3Mark
    > CgalNef3Plc;

class PlcNef3Internal {
public:
    template<typename... Args>
    PlcNef3Internal(Args&&... args) : p(args...) { }
    os2cx::CgalNef3Plc p;
};

} /* namespace os2cx */

#endif /* OS2CX_POLY_BITS_INTERNAL_HPP_ */
