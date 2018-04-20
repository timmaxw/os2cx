#ifndef OS2CX_CALCULIX_FRD_READ_HPP_
#define OS2CX_CALCULIX_FRD_READ_HPP_

#include <iostream>

#include "result.hpp"

namespace os2cx {

class CalculixFrdFileReadError : public std::runtime_error {
public:
    CalculixFrdFileReadError(const std::string &msg) :
        std::runtime_error(msg) { }
};

void read_calculix_frd(
    std::istream &stream,
    NodeId node_id_begin,
    NodeId node_id_end,
    Results *results_out);

} /* namespace os2cx */

#endif

