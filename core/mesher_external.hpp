#ifndef OS2CX_MESHER_EXTERNAL_HPP_
#define OS2CX_MESHER_EXTERNAL_HPP_

#include <stdexcept>
#include <string>

#include "mesh.hpp"
#include "poly.hpp"
#include "util.hpp"

namespace os2cx {

class ExternalMesherError : public std::runtime_error {
public:
    ExternalMesherError(const std::string &msg) : std::runtime_error(msg) { }
};

Mesh3 mesher_netgen(
    const Poly3 &poly3,
    const FilePath &temp_dir,
    const std::string &name);

} /* namespace os2cx */

#endif
