#ifndef OS2CX_BEACON_HPP_
#define OS2CX_BEACON_HPP_

#include "calc.hpp"
#include "poly.hpp"

namespace os2cx {

class BeaconError : std::runtime_error
{
public:
    BeaconError() : std::runtime_error("Invalid beacon") { }
};

AffineTransform recover_beacon(const Poly3 &beacon);

} /* namespace os2cx */

#endif /* OS2CX_BEACON_HPP_ */
