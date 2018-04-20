#ifndef OS2CX_CALCULIX_RUN_HPP_
#define OS2CX_CALCULIX_RUN_HPP_

#include <iostream>

#include "mesh.hpp"

namespace os2cx {

class CalculixRunError : public std::runtime_error {
public:
    CalculixRunError(const std::string &msg) :
        std::runtime_error(msg) { }
};

void run_calculix(
    const std::string &temp_dir,
    const std::string &filename);

} /* namespace os2cx */

#endif

