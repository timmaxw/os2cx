#include "project_run.hpp"

#include <fstream>

#include "calculix_frd_read.hpp"
#include "calculix_inp_write.hpp"
#include "calculix_run.hpp"
#include "mesher_naive_bricks.hpp"
#include "mesher_tetgen.hpp"
#include "openscad_extract.hpp"
#include "openscad_run.hpp"
#include "plc_nef_to_plc.hpp"

namespace os2cx {

void project_run(Project *p, ProjectRunCallbacks *callbacks) {
    maybe_create_directory(p->temp_dir);

    callbacks->project_run_log("Scanning OpenSCAD file...");
    try {
        openscad_extract_inventory(p);
    } catch (const OpenscadRunError &error) {
        callbacks->project_run_log("Error running OpenSCAD:");
        for (const std::string &error_line : error.errors) {
            callbacks->project_run_log(error_line);
        }
        p->errored = true;
        return;
    } catch (const UsageError &error) {
        callbacks->project_run_log("Error in OpenSCAD file:");
        callbacks->project_run_log(error.what());
        p->errored = true;
        return;
    } catch (const BadEchoError &error) {
        callbacks->project_run_log("Malformed echo from OpenSCAD:");
        callbacks->project_run_log(error.what());
        p->errored = true;
        return;
    }
    p->progress = Project::Progress::InventoryDone;
    callbacks->project_run_checkpoint();

    for (auto &pair : p->mesh_objects) {
        callbacks->project_run_log("Loading mesh '" + pair.first + "'...");
        pair.second.solid = openscad_extract_poly3(
            p, "mesh", pair.first);
        callbacks->project_run_checkpoint();
    }
    for (auto &pair : p->select_volume_objects) {
        callbacks->project_run_log("Loading volume '" + pair.first + "'...");
        pair.second.mask = openscad_extract_poly3(
            p, "select_volume", pair.first);
        callbacks->project_run_checkpoint();
    }
    for (auto &pair : p->select_surface_objects) {
        callbacks->project_run_log("Loading surface '" + pair.first + "'...");
        pair.second.mask = openscad_extract_poly3(
            p, "select_surface", pair.first);
        callbacks->project_run_checkpoint();
    }
    p->progress = Project::Progress::PolysDone;
    callbacks->project_run_checkpoint();

    for (auto &pair : p->mesh_objects) {
        callbacks->project_run_log(
            "Preprocessing mesh '" + pair.first + "'...");
        PlcNef3 solid_nef = compute_plc_nef_for_solid(*pair.second.solid);
        for (auto &select_volume_pair : p->select_volume_objects) {
            compute_plc_nef_select_volume(
                &solid_nef,
                *select_volume_pair.second.mask,
                select_volume_pair.second.bit_index);
        }
        for (auto &select_surface_pair : p->select_surface_objects) {
            compute_plc_nef_select_surface(
                &solid_nef,
                *select_surface_pair.second.mask,
                select_surface_pair.second.direction_vector,
                select_surface_pair.second.direction_angle_tolerance,
                select_surface_pair.second.bit_index);
        }

        pair.second.plc.reset(new Plc3(plc_nef_to_plc(solid_nef)));
        pair.second.plc_index.reset(new Plc3Index(pair.second.plc.get()));

        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->mesh_objects) {
        p->approx_scale = std::max(
            p->approx_scale,
            pair.second.plc_index->approx_scale());
    }
    p->progress = Project::Progress::PolyAttrsDone;
    callbacks->project_run_checkpoint();

    for (auto &pair : p->mesh_objects) {
        callbacks->project_run_log("Meshing '" + pair.first + "'...");
        double max_element_size = pair.second.max_element_size;
        if (max_element_size == Project::MeshObject::SUGGEST_MAX_ELEMENT_SIZE) {
            max_element_size = suggest_max_element_size(*pair.second.plc);
            callbacks->project_run_log("Automatically chose max_element_size=" +
                std::to_string(max_element_size));
        }

        switch(pair.second.mesher) {
        case Project::MeshObject::Mesher::Tetgen: {
            pair.second.partial_mesh.reset(new Mesh3(mesher_tetgen(
                *pair.second.plc,
                max_element_size
            )));
            break;
        }
        case Project::MeshObject::Mesher::NaiveBricks: {
            pair.second.partial_mesh.reset(new Mesh3(mesher_naive_bricks(
                *pair.second.plc,
                max_element_size
            )));
            break;
        }
        default: assert(false);
        }

        callbacks->project_run_checkpoint();
    }

    {
        callbacks->project_run_log("Merging meshes...");
        Mesh3 combined_mesh;
        for (auto &pair : p->mesh_objects) {
            combined_mesh.append_mesh(
                *pair.second.partial_mesh,
                &pair.second.node_begin,
                &pair.second.node_end,
                &pair.second.element_begin,
                &pair.second.element_end);
            pair.second.partial_mesh = nullptr;

            pair.second.element_set.reset(new ElementSet(
                compute_element_set_from_range(
                    pair.second.element_begin, pair.second.element_end)
            ));
            pair.second.node_set.reset(new NodeSet(
                compute_node_set_from_range(
                    pair.second.node_begin, pair.second.node_end)
            ));
        }
        p->mesh.reset(new Mesh3(std::move(combined_mesh)));
        p->mesh_index.reset(new Mesh3Index(p->mesh.get()));
        p->progress = Project::Progress::MeshDone;
        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->select_volume_objects) {
        callbacks->project_run_log("Computing volume '" + pair.first + "'...");
        ElementSet element_set;
        for (auto &mesh_pair : p->mesh_objects) {
            ElementSet partial_element_set = compute_element_set_from_plc_bit(
                *mesh_pair.second.plc_index,
                *p->mesh,
                mesh_pair.second.element_begin,
                mesh_pair.second.element_end,
                pair.second.bit_index);
            element_set.elements.insert(
                partial_element_set.elements.begin(),
                partial_element_set.elements.end());
        }

        NodeSet node_set =
            compute_node_set_from_element_set(*p->mesh, element_set);

        pair.second.element_set.reset(new ElementSet(std::move(element_set)));
        pair.second.node_set.reset(new NodeSet(std::move(node_set)));

        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->select_surface_objects) {
        callbacks->project_run_log("Computing surface '" + pair.first + "'...");
        FaceSet face_set;
        for (auto &mesh_pair : p->mesh_objects) {
            FaceSet partial_face_set = compute_face_set_from_plc_bit(
                *mesh_pair.second.plc_index,
                *p->mesh,
                *p->mesh_index,
                mesh_pair.second.element_begin,
                mesh_pair.second.element_end,
                pair.second.bit_index);
            face_set.faces.insert(
                partial_face_set.faces.begin(),
                partial_face_set.faces.end());
        }

        NodeSet node_set = compute_node_set_from_face_set(*p->mesh, face_set);

        pair.second.face_set.reset(new FaceSet(std::move(face_set)));
        pair.second.node_set.reset(new NodeSet(std::move(node_set)));

        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->load_volume_objects) {
        callbacks->project_run_log("Computing load '" + pair.first + "'...");
        const ElementSet &element_set =
            *p->find_volume_object(pair.second.volume)->element_set;
        pair.second.load.reset(new ConcentratedLoad(
            compute_load_from_element_set(
                *p->mesh,
                element_set,
                p->unit_system.unit_to_system(
                    pair.second.force_total_or_per_volume),
                pair.second.force_is_per_volume)
        ));
        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->load_surface_objects) {
        callbacks->project_run_log("Computing load '" + pair.first + "'...");
        const FaceSet &face_set =
            *p->find_surface_object(pair.second.surface)->face_set;
        pair.second.load.reset(new ConcentratedLoad(
            compute_load_from_face_set(
                *p->mesh,
                face_set,
                p->unit_system.unit_to_system(
                    pair.second.force_total_or_per_area),
                pair.second.force_is_per_area)
        ));
        callbacks->project_run_checkpoint();
    }

    p->progress = Project::Progress::MeshAttrsDone;
    callbacks->project_run_checkpoint();

    callbacks->project_run_log("Writing CalculiX input files...");
    write_calculix_job(p->temp_dir, "main", *p);
    callbacks->project_run_checkpoint();

    try {
        run_calculix(p->temp_dir, "main");
    } catch (const CalculixRunError &error) {
        callbacks->project_run_log("CalculiX failed.");
        p->errored = true;
        return;
    }

    callbacks->project_run_log("Reading CalculiX output files...");
    std::ifstream frd_stream(p->temp_dir + "/main.frd");
    std::vector<FrdAnalysis> frd_analyses;
    try {
        read_calculix_frd(
            frd_stream,
            p->mesh->nodes.key_begin(),
            p->mesh->nodes.key_end(),
            &frd_analyses);
    } catch (const CalculixFrdFileReadError &error) {
        callbacks->project_run_log("Error reading CalculiX output file:");
        callbacks->project_run_log(error.what());
        p->errored = true;
        return;
    }

    Results results;
    results_from_frd_analyses(frd_analyses, &results);
    p->results.reset(new Results(std::move(results)));
    p->progress = Project::Progress::ResultsDone;
    callbacks->project_run_log("Done.");
}

} /* namespace os2cx */

