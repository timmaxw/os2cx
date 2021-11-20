#ifndef OS2CX_OPENSCAD_EXTRACT_HPP_
#define OS2CX_OPENSCAD_EXTRACT_HPP_

#include <string>

#include "project.hpp"
#include "util.hpp"

namespace os2cx {

/* UsageError is thrown if the OpenSCAD2Calculix declarations in the user's
OpenSCAD script are somehow invalid. */
class UsageError : public std::runtime_error {
public:
    UsageError(const std::string &msg) : std::runtime_error(msg) { }
};

/* BadEchoError is thrown if the specially-formatted echo() calls we get from
the OpenSCAD script are incorrectly formatted. The distinction between this and
UsageError is that UsageError is the sort of thing that a novice user might do
by accident, but BadEchoError would require tampering with the internals of
"openscad2calculix.scad". */
class BadEchoError : public std::runtime_error {
public:
    BadEchoError(const std::string &msg) : std::runtime_error(msg) { }
};

void openscad_extract_inventory(Project *project);

std::unique_ptr<Poly3> openscad_extract_poly3(
    Project *project,
    const std::string &object_type,
    const std::string &name);

void openscad_process_deck(Project *project);

} /* namespace os2cx */

#endif
