#include "project.hpp"

#include <fstream>

#include "calculix_frd_read.hpp"
#include "calculix_inp_write.hpp"
#include "calculix_run.hpp"
#include "mesher_tetgen.hpp"
#include "openscad_extract.hpp"

namespace os2cx {

std::vector<const Poly3 *> get_volume_masks(const Project *project) {
    std::vector<const Poly3 *> volume_masks;
    for (auto it = project->select_volume_objects.begin();
            it != project->select_volume_objects.end(); ++it) {
        volume_masks.push_back(it->second.mask.get());
    }
    return volume_masks;
}

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
        std::shared_ptr<Poly3Map> poly3_map(new Poly3Map);
        poly3_map_create(
            *pair.second.solid,
            get_volume_masks(p),
            poly3_map.get()
        );
        pair.second.poly3_map = poly3_map;
        pair.second.poly3_map_index.reset(new Poly3MapIndex(*poly3_map));
        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->mesh_objects) {
        Mesh3 partial_mesh = mesher_tetgen(*pair.second.poly3_map);
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

            Project::VolumeObject *volume_object = &p->volume_objects[pair.first];
            volume_object->element_set.reset(new ElementSet(
                compute_element_set_from_range(
                    pair.second.element_begin, pair.second.element_end)
            ));
            volume_object->node_set.reset(new NodeSet(
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
            ElementSet partial_element_set = compute_element_set_from_mask(
                *mesh_pair.second.poly3_map,
                *mesh_pair.second.poly3_map_index,
                *p->mesh,
                mesh_pair.second.element_begin,
                mesh_pair.second.element_end,
                pair.second.mask.get());
            element_set.elements.insert(
                partial_element_set.elements.begin(),
                partial_element_set.elements.end());
        }

        NodeSet node_set =
            compute_node_set_from_element_set(*p->mesh, element_set);

        Project::VolumeObject *vol = &p->volume_objects[pair.first];
        vol->element_set.reset(new ElementSet(std::move(element_set)));
        vol->node_set.reset(new NodeSet(std::move(node_set)));

        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->load_objects) {
        const ElementSet &element_set =
            *p->volume_objects[pair.second.volume].element_set;
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

