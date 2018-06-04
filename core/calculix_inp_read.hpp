#ifndef OS2CX_CALCULIX_INP_READ_HPP_
#define OS2CX_CALCULIX_INP_READ_HPP_

#include <iostream>

#include "mesh.hpp"

namespace os2cx {

class CalculixFileReadError : public std::runtime_error {
public:
    CalculixFileReadError(const std::string &msg) :
        std::runtime_error(msg) { }
};

void read_calculix_nodes_and_elements(
    std::istream &stream,
    Mesh3 *mesh_out);

} /* namespace os2cx */

#endif

