#include "calculix_inp_write.hpp"

#include <fstream>

namespace os2cx {

void write_calculix_nodes_and_elements(
    std::ostream &stream,
    const std::string &name,
    const Mesh3 &mesh
) {
    stream << "*NODE, NSET=N" << name << '\n';
    for (NodeId nid = mesh.nodes.key_begin();
            nid != mesh.nodes.key_end(); ++nid) {
        const Node3 &node = mesh.nodes[nid];
        stream << nid.to_int()
            << ", " << node.point.vector.x.val
            << ", " << node.point.vector.y.val
            << ", " << node.point.vector.z.val << '\n';
    }

    std::set<ElementType> types_present;
    for (const Element3 &element : mesh.elements) {
        types_present.insert(element.type);
    }

    for (ElementType type : types_present) {
        const ElementTypeInfo &type_info = ElementTypeInfo::get(type);
        stream << "*ELEMENT, TYPE=" << type_info.name
            << ", ELSET=E" << name << '\n';
        for (ElementId eid = mesh.elements.key_begin();
                eid != mesh.elements.key_end(); ++eid) {
            const Element3 *element = &mesh.elements[eid];
            if (element->type != type) continue;
            stream << eid.to_int();
            for (size_t i = 0; i < type_info.shape->vertices.size(); ++i) {
                stream << ", " << element->nodes[i].to_int();
            }
            stream << '\n';
        }
    }
}

void write_calculix_nset(
    std::ostream &stream,
    const std::string &name,
    const NodeSet &node_set
) {
    stream << "*NSET, NSET=N" << name << '\n';
    for (NodeId node_id : node_set.nodes) {
        stream << node_id.to_int() << "\n";
    }
}

void write_calculix_cload(
    std::ostream &stream,
    const ConcentratedLoad &cload
) {
    for (const auto &pair : cload.loads) {
        const ForceVector &force = pair.second.force;
        if (force.x != Force(0)) {
            stream << pair.first.to_int() << ",1," << force.x.val << std::endl;
        }
        if (force.y != Force(0)) {
            stream << pair.first.to_int() << ",2," << force.y.val << std::endl;
        }
        if (force.z != Force(0)) {
            stream << pair.first.to_int() << ",3," << force.z.val << std::endl;
        }
    }
}

void write_calculix_job(
    const FilePath &dir_path,
    const std::string &main_file_name,
    const Project &project
) {
    FilePath main_file_path = dir_path + "/" + main_file_name + ".inp";
    std::ofstream main_stream(main_file_path);

    for (auto it = project.element_directives.begin();
            it != project.element_directives.end(); ++it) {
        FilePath mesh_file_path = dir_path + "/" + it->first + ".msh";
        std::ofstream mesh_stream(mesh_file_path);
        write_calculix_nodes_and_elements(
            mesh_stream, it->first, *it->second.mesh);
        main_stream << "*INCLUDE, INPUT=" << it->first << ".msh\n";
    }

    for (auto it = project.nset_directives.begin();
            it != project.nset_directives.end(); ++it) {
        FilePath nset_file_path = dir_path + "/" + it->first + ".nam";
        std::ofstream nset_stream(nset_file_path);
        write_calculix_nset(
            nset_stream, it->first, *it->second.node_set);
        main_stream << "*INCLUDE, INPUT=" << it->first << ".nam\n";
    }

    if (project.total_cload) {
        FilePath cload_file_path = dir_path + "/load.clo";
        std::ofstream cload_stream(cload_file_path);
        write_calculix_cload(
            cload_stream, *project.total_cload);
    }

    for (const std::string &directive : project.directives) {
        main_stream << directive << '\n';
    }
}

} /* namespace os2cx */

