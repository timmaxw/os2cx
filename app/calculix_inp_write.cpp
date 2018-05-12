#include "calculix_inp_write.hpp"

#include <fstream>

namespace os2cx {

void write_calculix_nodes_and_elements(
    std::ostream &stream,
    const std::string &name,
    const Mesh3 &mesh,
    NodeId node_begin,
    NodeId node_end,
    ElementId element_begin,
    ElementId element_end
) {
    stream << "*NODE, NSET=N" << name << '\n';
    for (NodeId nid = node_begin; nid != node_end; ++nid) {
        const Node3 &node = mesh.nodes[nid];
        stream << nid.to_int()
            << ", " << node.point.vector.x.val
            << ", " << node.point.vector.y.val
            << ", " << node.point.vector.z.val << '\n';
    }

    std::set<ElementType> types_present;
    for (ElementId eid = element_begin; eid != element_end; ++eid) {
        types_present.insert(mesh.elements[eid].type);
    }

    for (ElementType type : types_present) {
        const ElementTypeInfo &type_info = ElementTypeInfo::get(type);
        stream << "*ELEMENT, TYPE=" << type_info.name
            << ", ELSET=E" << name << '\n';
        for (ElementId eid = element_begin; eid != element_end; ++eid) {
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

void write_calculix_elset(
    std::ostream &stream,
    const std::string &name,
    const ElementSet &element_set
) {
    stream << "*ELSET, ELSET=E" << name << '\n';
    for (ElementId element_id : element_set.elements) {
        stream << element_id.to_int() << "\n";
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
    for (const auto &pair : project.mesh_objects) {
        FilePath mesh_file_path = dir_path + "/" + pair.first + ".msh";
        std::ofstream mesh_stream(mesh_file_path);
        write_calculix_nodes_and_elements(
            mesh_stream, pair.first, *project.mesh,
            pair.second.node_begin, pair.second.node_end,
            pair.second.element_begin, pair.second.element_end);
    }

    for (const auto &pair : project.select_volume_objects) {
        FilePath select_file_path = dir_path + "/" + pair.first + ".nam";
        std::ofstream select_stream(select_file_path);
        write_calculix_nset(
            select_stream, pair.first, *pair.second.node_set);
        write_calculix_elset(
            select_stream, pair.first, *pair.second.element_set);
    }

    for (const auto &pair : project.load_objects) {
        FilePath load_file_path = dir_path + "/" + pair.first + ".clo";
        std::ofstream load_stream(load_file_path);
        write_calculix_cload(load_stream, *pair.second.load);
    }

    FilePath main_file_path = dir_path + "/" + main_file_name + ".inp";
    std::ofstream main_stream(main_file_path);
    for (const std::string &line : project.calculix_deck) {
        main_stream << line << '\n';
    }
}

} /* namespace os2cx */

