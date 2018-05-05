#ifndef OS2CX_OPENSCAD_RUN_HPP_
#define OS2CX_OPENSCAD_RUN_HPP_

#include <map>
#include <memory>

#include "openscad_value.hpp"
#include "poly.hpp"
#include "util.hpp"

namespace TinyProcessLib {
class Process;
}

namespace os2cx {

/* OpenscadRunError indicates that the Openscad invocation failed because
there was an error in the Openscad script. */
class OpenscadRunError : public std::runtime_error {
public:
    OpenscadRunError() : std::runtime_error("OpenSCAD failed") { }
    std::vector<std::string> errors;
};

class OpenscadRun {
public:
    OpenscadRun();
    OpenscadRun(
        const FilePath &input_path,
        const FilePath &geometry_path,
        const std::map<std::string, OpenscadValue> &defines);
    ~OpenscadRun();

    void wait();

    std::vector<std::vector<OpenscadValue> > echos;
    std::vector<std::string> warnings, errors;

    std::string geometry_path;
    std::unique_ptr<Poly3> geometry;

private:
    void handle_output_chunk(const char *bytes, size_t n);
    void handle_output_line(const char *begin, const char *end);

    std::unique_ptr<TinyProcessLib::Process> process;
    std::vector<char> output_leftover;
    bool has_geometry;
};

} /* namespace os2cx */

#endif
