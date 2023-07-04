#ifndef OS2CX_POLY_HPP_
#define OS2CX_POLY_HPP_

#include <iostream>
#include <memory>
#include <vector>

#include "calc.hpp"

namespace os2cx {

/* Poly3 is a closed polyhedron with an inside and an outside. It may have
multiple disconnected parts. Not to be confused with Mesh3; a Poly3 only defines
a surface, and uses triangles of whatever size is most convenient, whereas a
Mesh3 fills a volume with tetrahedra or other elements, and subdivides them to a
size that's good for simulation. */

/* Poly3 is internally implemented as a CGAL::Polyhedron_3, but CGAL headers
take a long time to compile, so the implementation is hidden in Poly3Internal.
All of the facets are guaranteed to be triangles. */
class Poly3Internal;

class Poly3 {
public:
    /* Creates a box-shaped polyhedron. Mostly for testing. */
    static Poly3 from_box(const Box &box);

    /* Creates a polyhedron from several box-shaped regions. "invert" must have
    the same length as "boxes"; if "invert[i]" is true, then "boxes[i]" will be
    treated as a hole. The boxes must not overlap, unless invert[i]=true, in
    which case that box must be entirely contained within another box. This is
    for testing handling of more complex polyhedra. */
    static Poly3 from_boxes(const std::vector<Box> &boxes, const std::vector<bool> &invert);

    Poly3();
    Poly3(Poly3 &&other);
    ~Poly3();
    Poly3 &operator=(Poly3 &&other);

    std::unique_ptr<Poly3Internal> i;
};

class PolyIoError : public std::runtime_error {
public:
    PolyIoError(const std::string &msg) :
        std::runtime_error(msg) { }
};

Poly3 read_poly3_off(
    std::istream &stream);

void write_poly3_off(
    std::ostream &stream, const Poly3 &poly);

void write_poly3_stl_text(
    std::ostream &stream, const Poly3 &poly);

} /* namespace os2cx */

#endif

