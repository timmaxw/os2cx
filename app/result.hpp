#ifndef OS2CX_RESULT_HPP_
#define OS2CX_RESULT_HPP_

#include <map>
#include <string>

#include "mesh.hpp"

namespace os2cx {

class Results {
public:
    std::map<std::string, ContiguousMap<NodeId, PureVector> > node_vectors;
};

} /* namespace os2cx */

#endif
