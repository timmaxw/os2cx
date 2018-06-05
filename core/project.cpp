#include "project.hpp"

#include <fstream>

#include "calculix_frd_read.hpp"
#include "calculix_inp_write.hpp"
#include "calculix_run.hpp"
#include "mesher_tetgen.hpp"
#include "openscad_extract.hpp"
#include "plc_nef_to_plc.hpp"

namespace os2cx {

void project_run(Project *p, ProjectRunCallbacks *callbacks) {
    maybe_create_directory(p->temp_dir);
    openscad_extract_inventory(p);
    callbacks->project_run_checkpoint();

    for (auto &pair : p->mesh_objects) {
        pair.second.solid = openscad_extract_poly3(
            p, "mesh", pair.first);
        callbacks->project_run_checkpoint();
    }
    for (auto &pair : p->select_volume_objects) {
        pair.second.mask = openscad_extract_poly3(
            p, "select_volume", pair.first);
        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->mesh_objects) {
        PlcNef3 solid_nef = compute_plc_nef_for_solid(*pair.second.solid);
        for (auto &select_volume_pair : p->select_volume_objects) {
            compute_plc_nef_select_volume(
                &solid_nef,
                *select_volume_pair.second.mask,
                select_volume_pair.second.bit_index);
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
    callbacks->project_run_checkpoint();

    for (auto &pair : p->mesh_objects) {
        Mesh3 partial_mesh = mesher_tetgen(*pair.second.plc);
        pair.second.partial_mesh.reset(new Mesh3(std::move(partial_mesh)));
        callbacks->project_run_checkpoint();
    }

    {
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
        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->select_volume_objects) {
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

    for (auto &pair : p->load_objects) {
        const ElementSet &element_set =
            *p->find_volume_object(pair.second.volume)->element_set;
        pair.second.load.reset(new ConcentratedLoad(
            compute_load_from_element_set(
                *p->mesh,
                element_set,
                pair.second.force_density)
        ));
        callbacks->project_run_checkpoint();
    }

    write_calculix_job(p->temp_dir, "main", *p);
    callbacks->project_run_checkpoint();

    run_calculix(p->temp_dir, "main");
    Results results;
    std::ifstream frd_stream(p->temp_dir + "/main.frd");
    read_calculix_frd(
        frd_stream,
        p->mesh->nodes.key_begin(),
        p->mesh->nodes.key_end(),
        &results);
    p->results.reset(new Results(std::move(results)));
    callbacks->project_run_checkpoint();
}

} /* namespace os2cx */

