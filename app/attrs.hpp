#ifndef OS2CX_ATTRS_HPP_
#define OS2CX_ATTRS_HPP_

#include "mesh.hpp"
#include "region.hpp"
#include "region_map.hpp"
#include "region_map_index.hpp"

namespace os2cx {

class NodeSet {
public:
    std::set<NodeId> nodes;
};

void node_set_volume(
    const RegionMap3 &region_map,
    const RegionMap3Index &region_map_index,
    const Mesh3 &mesh,
    const Region3 *mask,
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
    const RegionMap3 &region_map,
    const RegionMap3Index &region_map_index,
    const Mesh3 &mesh,
    const Region3 *mask,
    ForceDensityVector force,
    ConcentratedLoad *load_out);

} /* namespace os2cx */

#endif

