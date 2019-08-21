#ifndef OS2CX_ATTRS_HPP_
#define OS2CX_ATTRS_HPP_

#include <bitset>

namespace os2cx {

static const int num_attr_bits = 64;
typedef int AttrBitIndex;
typedef std::bitset<num_attr_bits> AttrBitset;

AttrBitIndex attr_bit_solid();

} /* namespace os2cx */

#endif
