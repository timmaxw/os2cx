#ifndef OS2CX_ATTRS_HPP_
#define OS2CX_ATTRS_HPP_

#include <bitset>
#include <cassert>
#include <vector>

namespace os2cx {

static const int num_attr_bits = 64;
typedef int AttrBitIndex;
typedef std::bitset<num_attr_bits> AttrBitset;

AttrBitIndex attr_bit_solid();

template<class T>
class AttrOverrides {
public:
    void add(AttrBitIndex attr_index, T value) {
        assert(!overridden_attrs[attr_index]);
        overridden_attrs.set(attr_index);
        values[attr_index] = value;
    }

    T lookup(AttrBitset attrs, T default_value) const {
        for (AttrBitIndex i = num_attr_bits - 1; i >= 0; --i) {
            if (attrs[i] && overridden_attrs[i]) {
                return values[i];
            }
        }
        return default_value;
    }

    AttrBitset overridden_attrs;
    T values[num_attr_bits];
};

typedef double MaxElementSize;
typedef int MaterialId;

} /* namespace os2cx */

#endif
