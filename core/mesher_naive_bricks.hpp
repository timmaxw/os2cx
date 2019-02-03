#ifndef OS2CX_MESHER_NAIVE_BRICKS_HPP_
#define OS2CX_MESHER_NAIVE_BRICKS_HPP_

#include "mesh.hpp"
#include "plc.hpp"

namespace os2cx {

class NaiveBricksAlignmentError : public std::exception {
public:
    NaiveBricksAlignmentError(Point _p0, Point _p1) :
        is_triangle(false), p0(_p0), p1(_p1) { }
    NaiveBricksAlignmentError(Point _p0, Point _p1, Point _p2) :
        is_triangle(true), p0(_p0), p1(_p1), p2(_p2) { }
    bool is_triangle;
    Point p0, p1, p2;
};

Mesh3 mesher_naive_bricks(
    const Plc3 &plc,
    double max_element_size);

} /* namespace os2cx */

#endif
