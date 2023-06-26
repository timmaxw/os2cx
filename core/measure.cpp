#include "measure.hpp"

#include "project.hpp"

namespace os2cx {

double Measure::measure(
    const Project &project,
    const Results::Result::Step &step
) const {
    auto dataset_it = step.datasets.find(dataset);
    if (dataset_it == step.datasets.end()) {
        return NAN;
    }
    const Results::Dataset &dataset_obj = dataset_it->second;

    SubVariable measure_subvariable;
    if (dataset_obj.node_scalar) {
        measure_subvariable = SubVariable::ScalarValue;
    } else if (dataset_obj.node_vector) {
        measure_subvariable = SubVariable::VectorMagnitude;
    } else if (dataset_obj.node_complex_vector) {
        measure_subvariable = SubVariable::ComplexVectorMagnitude;
    } else if (dataset_obj.node_matrix) {
        measure_subvariable = SubVariable::MatrixVonMisesStress;
    } else {
        assert(false);
    }

    std::shared_ptr<const NodeSet> node_set;
    const Project::VolumeObject *volume;
    const Project::SurfaceObject *surface;
    const Project::NodeObject *node;
    if ((volume = project.find_volume_object(subject))) {
        node_set = volume->node_set;
    } else if ((surface = project.find_surface_object(subject))) {
        node_set = surface->node_set;
    } else if ((node = project.find_node_object(subject))) {
        node_set.reset(new NodeSet(
            compute_node_set_singleton(node->node_id)));
    }

    double max_datum = 0;
    for (NodeId node_id : node_set->nodes) {
        double datum = dataset_obj.subvariable_value(
            measure_subvariable,
            node_id);
        if (isnan(datum)) {
            return NAN;
        }
        max_datum = std::max(datum, max_datum);
    }
    return max_datum;
}

} // namespace os2cx
