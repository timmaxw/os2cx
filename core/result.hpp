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
    class Variable {
    public:
        /* Exactly one of these will be non-null */
        std::unique_ptr<ContiguousMap<NodeId, Vector> > node_vector;
        std::unique_ptr<ContiguousMap<NodeId, Matrix> > node_matrix;
    };
    class VariableSet {
    public:
        VariableSet() : has_disp(false) { }
        std::map<std::string, Variable> vars;
        bool has_disp; /* The DISP variable is given special treatment */
    };
    std::map<std::string, VariableSet> static_steps;
};

void results_from_frd_analyses(
    const std::vector<FrdAnalysis> &frd_analyses,
    Results *results_out);

} /* namespace os2cx */

#endif
