#ifndef OS2CX_ATTRS_HPP_
#define OS2CX_ATTRS_HPP_

#include "mesh.hpp"
#include "poly.hpp"
#include "poly_map.hpp"
#include "poly_map_index.hpp"

namespace os2cx {

class NodeSet {
public:
    std::set<NodeId> nodes;
};

void node_set_volume(
    const Poly3Map &poly3_map,
    const Poly3MapIndex &poly3_map_index,
    const Mesh3 &mesh,
    const Poly3 *mask,
    NodeSet *nset_out);

class ConcentratedLoad {
public:
    class Load {
    public:
        Load() : force(ForceVector::zero()) { }
        ForceVector force;
    };
    std::map<NodeId, Load> loads;
};

void load_volume(
    const Poly3Map &poly3_map,
    const Poly3MapIndex &poly3_map_index,
    const Mesh3 &mesh,
    const Poly3 *mask,
    ForceDensityVector force,
    ConcentratedLoad *load_out);

} /* namespace os2cx */

#endif

