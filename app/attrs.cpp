#include "attrs.hpp"

namespace os2cx {

template<class Callable>
void for_element_in_mask(
    const RegionMap3 &region_map,
    const RegionMap3Index &region_map_index,
    const Mesh3 &mesh,
    const Region3 *mask,
    const Callable &callable
) {
    for (const Element3 &element : mesh.elements) {
        LengthVector c = LengthVector::zero();
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            c += mesh.nodes[element.nodes[i]].point.vector;
        }
        c /= num_nodes;
        const RegionMap3::Volume &volume = region_map.volumes[
            region_map_index.volume_containing_point(Point(c))];
        assert(volume.is_solid);
        if (volume.masks.find(mask)->second) {
            callable(element);
        }
    }
}

void node_set_volume(
    const RegionMap3 &region_map,
    const RegionMap3Index &region_map_index,
    const Mesh3 &mesh,
    const Region3 *mask,
    NodeSet *nset_out
) {
    for_element_in_mask(region_map, region_map_index, mesh, mask,
    [&](const Element3 &element) {
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            nset_out->nodes.insert(element.nodes[i]);
        }
    });
}

void load_volume(
    const RegionMap3 &region_map,
    const RegionMap3Index &region_map_index,
    const Mesh3 &mesh,
    const Region3 *mask,
    ForceDensityVector force,
    ConcentratedLoad *load_out
) {
    for_element_in_mask(region_map, region_map_index, mesh, mask,
    [&](const Element3 &element) {
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            Volume vol = mesh.volume_for_node(element, i);
            load_out->loads[element.nodes[i]].force += vol * force;
        }
    });
}

} /* namespace os2cx */

