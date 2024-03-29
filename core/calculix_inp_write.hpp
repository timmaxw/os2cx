#ifndef OS2CX_CALCULIX_INP_WRITE_HPP_
#define OS2CX_CALCULIX_INP_WRITE_HPP_

#include <iostream>

#include "project.hpp"
#include "util.hpp"

namespace os2cx {

void write_calculix_create_node(
    std::ostream &stream,
    const std::string &name,
    NodeId node_id,
    Point point);

void write_calculix_nodes_and_elements(
    std::ostream &stream,
    const std::string &name,
    const Mesh3 &mesh,
    NodeId node_begin,
    NodeId node_end,
    ElementId element_begin,
    ElementId element_end);

void write_calculix_nset(
    std::ostream &stream,
    const std::string &name,
    const NodeSet &node_set);

void write_calculix_elset(
    std::ostream &stream,
    const std::string &name,
    const ElementSet &element_set);

void write_calculix_surface(
    std::ostream &stream,
    const std::string &name,
    const FaceSet &face_set);

void write_calculix_cload(
    std::ostream &stream,
    const ConcentratedLoad &cload);

void write_calculix_material(
    std::ostream &stream,
    const std::string &name,
    const Project::MaterialObject &material,
    const Project &project);

void write_calculix_job(
    const FilePath &dir_path,
    const std::string &main_file_name,
    const Project &project);

} /* namespace os2cx */

#endif
