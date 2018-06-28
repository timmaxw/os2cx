#ifndef OS2CX_RESULT_HPP_
#define OS2CX_RESULT_HPP_

#include <map>
#include <string>

#include "calculix_frd_read.hpp"
#include "mesh.hpp"

namespace os2cx {

class Results {
public:
    std::map<std::string, ContiguousMap<NodeId, Vector> > node_vectors;
};

void results_from_frd_analyses(
    const std::vector<FrdAnalysis> &frd_analyses,
    Results *results_out);

} /* namespace os2cx */

#endif
