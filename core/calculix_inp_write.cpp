#include "calculix_inp_write.hpp"

#include <fstream>

namespace os2cx {

void write_calculix_create_node(
    std::ostream &stream,
    const std::string &name,
    NodeId node_id,
    Point point
) {
    stream << "*NODE, NSET=N" << name << '\n';
    stream << node_id.to_int()
        << ", " << point.x
        << ", " << point.y
        << ", " << point.z
        << '\n';
}

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
            << ", " << node.point.x
            << ", " << node.point.y
            << ", " << node.point.z
            << '\n';
    }

    std::set<ElementType> types_present;
    for (ElementId eid = element_begin; eid != element_end; ++eid) {
        types_present.insert(mesh.elements[eid].type);
    }

    for (ElementType type : types_present) {
        const ElementTypeShape &shape = element_type_shape(type);
        stream << "*ELEMENT, TYPE=" << shape.name
            << ", ELSET=E" << name << '\n';
        for (ElementId eid = element_begin; eid != element_end; ++eid) {
            const Element3 *element = &mesh.elements[eid];
            if (element->type != type) continue;
            stream << eid.to_int();
            for (size_t i = 0; i < shape.vertices.size(); ++i) {
                if (i == 15) {
                    /* If there would be more than 16 entries on a single line,
                    CalculiX expects it to be split into two lines */
                    stream << "\n";
                } else {
                    stream << ", ";
                }
                stream << element->nodes[i].to_int();
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


void write_calculix_surface(
    std::ostream &stream,
    const std::string &name,
    const FaceSet &face_set
) {
    stream << "*SURFACE,NAME=S" << name << ",TYPE=ELEMENT\n";
    for (FaceId face_id : face_set.faces) {
      /* Note we number faces starting from 0, but CalculiX numbers faces starting
      from 1, which is why we write "face_id.face + 1". */
      stream << face_id.element_id.to_int() << ",S" << face_id.face + 1 << '\n';
    }
}

void write_calculix_cload(
    std::ostream &stream,
    const ConcentratedLoad &cload
) {
    for (const auto &pair : cload.loads) {
        const Vector &force = pair.second.force;
        if (force.x != 0) {
            stream << pair.first.to_int() << ",1," << force.x << std::endl;
        }
        if (force.y != 0) {
            stream << pair.first.to_int() << ",2," << force.y << std::endl;
        }
        if (force.z != 0) {
            stream << pair.first.to_int() << ",3," << force.z << std::endl;
        }
    }
}

void write_calculix_material_and_solid_section(
    std::ostream &stream,
    const std::string &name,
    const Project::MaterialObject &material,
    const Project &project
) {
    stream << "*MATERIAL, Name=" << name << '\n'
        << "*ELASTIC\n"
        << project.unit_system.unit_to_system(material.youngs_modulus)
        << ", " << material.poissons_ratio << '\n'
        << "*DENSITY\n"
        << project.unit_system.unit_to_system(material.density) << '\n';

    bool any_element = false;
    for (const auto &pair : project.mesh_objects) {
        for (ElementId eid = pair.second.element_begin;
                eid != pair.second.element_end; ++eid) {
            MaterialId element_material = project.material_overrides.lookup(
                project.mesh->elements[eid].attrs,
                pair.second.material);
            if (element_material != material.id) {
                continue;
            }
            if (!any_element) {
                stream << "*ELSET, ELSET=M" << name << '\n';
                any_element = true;
            }
            stream << eid.to_int() << "\n";
        }
    }

    if (any_element) {
        stream << "*SOLID SECTION,"
            << "MATERIAL=" << name << ','
            << "ELSET=M" << name << '\n';
    }
}

inline int dimension_to_int(Dimension d){
    switch (d) {
    case Dimension::X: return 1;
    case Dimension::Y: return 2;
    case Dimension::Z: return 3;
    default: assert(false);
    }
}

void write_calculix_equation(
    std::ostream &stream,
    const LinearEquation &equation,
    std::set<LinearEquation::Variable> *variables_used
) {
    /* Use the variable with the largest coefficient as the dependent variable,
    as long as it's not already used */
    LinearEquation::Variable dependent_variable(
        NodeId::invalid(), Dimension::X);
    double dependent_coefficient = 0;
    int num_nonzero_terms = 0;
    for (const auto &term : equation.terms) {
        if (term.second == 0) continue;
        ++num_nonzero_terms;
        if (fabs(term.second) > fabs(dependent_coefficient)
                && !variables_used->count(term.first)) {
            dependent_variable = term.first;
            dependent_coefficient = term.second;
        }
    }

    assert(dependent_variable.node_id != NodeId::invalid());

    stream << "*EQUATION\n" << num_nonzero_terms << '\n';
    stream << dependent_variable.node_id.to_int() << ','
        << dimension_to_int(dependent_variable.dimension) << ','
        << dependent_coefficient << '\n';
    for (const auto &term : equation.terms) {
        if (term.first == dependent_variable) continue;
        if (term.second == 0) continue;
        stream << term.first.node_id.to_int() << ','
            << dimension_to_int(term.first.dimension) << ','
            << term.second << '\n';
    }
    stream << '\n';

    variables_used->insert(dependent_variable);
}

void write_calculix_job(
    const FilePath &dir_path,
    const std::string &main_file_name,
    const Project &project
) {
    {
        FilePath geometry_file_path = dir_path + "/objects.inp";
        std::ofstream geometry_stream(geometry_file_path);

        for (const auto &pair : project.create_node_objects) {
            write_calculix_create_node(
                geometry_stream, pair.first,
                pair.second.node_id, pair.second.point);
        }

        for (const auto &pair : project.mesh_objects) {
            write_calculix_nodes_and_elements(
                geometry_stream, pair.first, *project.mesh,
                pair.second.node_begin, pair.second.node_end,
                pair.second.element_begin, pair.second.element_end);
        }

        std::set<LinearEquation::Variable> variables_used;
        for (const auto &pair : project.slice_objects) {
            for (const LinearEquation &equation : *pair.second.equations) {
                write_calculix_equation(
                    geometry_stream, equation, &variables_used);
            }
        }

        for (const auto &pair : project.select_volume_objects) {
            write_calculix_nset(
                geometry_stream, pair.first, *pair.second.node_set);
            write_calculix_elset(
                geometry_stream, pair.first, *pair.second.element_set);
        }

        for (const auto &pair : project.select_surface_objects) {
            write_calculix_nset(
                geometry_stream, pair.first, *pair.second.node_set);
            write_calculix_surface(
                geometry_stream, pair.first, *pair.second.face_set);
        }

        for (const auto &pair : project.select_node_objects) {
            write_calculix_nset(
                geometry_stream,
                pair.first,
                compute_node_set_singleton(pair.second.node_id)
                );
        }

        for (const auto &pair : project.material_objects) {
            write_calculix_material_and_solid_section(
                geometry_stream, pair.first, pair.second, project);
        }
    }

    for (const auto &pair : project.load_volume_objects) {
        FilePath load_file_path = dir_path + "/" + pair.first + ".clo";
        std::ofstream load_stream(load_file_path);
        write_calculix_cload(load_stream, *pair.second.load);
    }

    for (const auto &pair : project.load_surface_objects) {
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

