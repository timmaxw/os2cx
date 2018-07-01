#ifndef OS2CX_RESULT_HPP_
#define OS2CX_RESULT_HPP_

#include <map>
#include <memory>
#include <string>

#include "calculix_frd_read.hpp"
#include "mesh.hpp"

namespace os2cx {

class Results {
public:
    class Dataset {
    public:
        /* Exactly one of these will be non-null */
        std::unique_ptr<ContiguousMap<NodeId, Vector> > node_vector;
        std::unique_ptr<ContiguousMap<NodeId, Matrix> > node_matrix;
    };

    class StaticStep {
    public:
        std::map<std::string, Dataset> datasets;

        /* disp_key is the key in 'vars' under which the displacement variable
        is stored, or an empty string if there's no displacement variable in
        this VariableSet */
        std::string disp_key;
    };
    std::map<std::string, StaticStep> static_steps;
};

void results_from_frd_analyses(
    const std::vector<FrdAnalysis> &frd_analyses,
    Results *results_out);

} /* namespace os2cx */

#endif
