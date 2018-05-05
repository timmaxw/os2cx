#include "attrs.hpp"

namespace os2cx {

template<class Callable>
void for_element_in_mask(
    const Poly3Map &poly3_map,
    const Poly3MapIndex &poly3_map_index,
    const Mesh3 &mesh,
    const Poly3 *mask,
    const Callable &callable
) {
    for (const Element3 &element : mesh.elements) {
        LengthVector c = LengthVector::zero();
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            c += mesh.nodes[element.nodes[i]].point.vector;
        }
        c /= num_nodes;
        const Poly3Map::Volume &volume = poly3_map.volumes[
            poly3_map_index.volume_containing_point(Point(c))];
        assert(volume.is_solid);
        if (volume.masks.find(mask)->second) {
            callable(element);
        }
    }
}

void node_set_volume(
    const Poly3Map &poly3_map,
    const Poly3MapIndex &poly3_map_index,
    const Mesh3 &mesh,
    const Poly3 *mask,
    NodeSet *nset_out
) {
    for_element_in_mask(poly3_map, poly3_map_index, mesh, mask,
    [&](const Element3 &element) {
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            nset_out->nodes.insert(element.nodes[i]);
        }
    });
}

void load_volume(
    const Poly3Map &poly3_map,
    const Poly3MapIndex &poly3_map_index,
    const Mesh3 &mesh,
    const Poly3 *mask,
    ForceDensityVector force,
    ConcentratedLoad *load_out
) {
    for_element_in_mask(poly3_map, poly3_map_index, mesh, mask,
    [&](const Element3 &element) {
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            Volume vol = mesh.volume_for_node(element, i);
            load_out->loads[element.nodes[i]].force += vol * force;
        }
    });
}

} /* namespace os2cx */

