#ifndef OS2CX_BEACON_HPP_
#define OS2CX_BEACON_HPP_

#include "calc.hpp"
#include "poly.hpp"

namespace os2cx {

/* AffineTransform represents a transformation consisting of first multiplying
all coordinates by "matrix" (where matrix[0] is the first column, etc.) and then
adding "translation". */
class AffineTransform {
public:
    LengthVector matrix[3];
    LengthVector translation;
};

class BeaconError : std::runtime_error
{
public:
    BeaconError() : std::runtime_error("Invalid beacon") { }
};

AffineTransform recover_beacon(const Poly3 &beacon);

} /* namespace os2cx */

#endif /* OS2CX_BEACON_HPP_ */
