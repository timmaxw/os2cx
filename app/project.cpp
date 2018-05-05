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
    for (auto it = project->nset_directives.begin();
            it != project->nset_directives.end(); ++it) {
        volume_masks.push_back(it->second.mask.get());
    }
    for (auto it = project->volume_load_directives.begin();
            it != project->volume_load_directives.end(); ++it) {
        volume_masks.push_back(it->second.mask.get());
    }
    return volume_masks;
}

void project_run(Project *p, ProjectRunCallbacks *callbacks) {
    maybe_create_directory(p->temp_dir);
    openscad_extract_inventory(p);
    callbacks->project_run_checkpoint();

    for (auto &pair : p->element_directives) {
        pair.second.solid = openscad_extract_poly3(p, "element", pair.first);
        callbacks->project_run_checkpoint();
    }
    for (auto &pair : p->nset_directives) {
        pair.second.mask = openscad_extract_poly3(p, "nset", pair.first);
        callbacks->project_run_checkpoint();
    }
    for (auto &pair : p->volume_load_directives) {
        pair.second.mask = openscad_extract_poly3(p, "volume_load", pair.first);
        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->element_directives) {
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

    for (auto &pair : p->element_directives) {
        Mesh3 mesh = mesher_tetgen(*pair.second.poly3_map);
        pair.second.mesh.reset(new Mesh3(
            p->mesh_id_allocator.allocate(std::move(mesh))));
        pair.second.mesh_index.reset(new Mesh3Index(pair.second.mesh.get()));
        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->nset_directives) {
        std::shared_ptr<NodeSet> node_set(new NodeSet);
        for (auto it = p->element_directives.begin();
                it != p->element_directives.end(); ++it) {
            node_set_volume(
                *it->second.poly3_map,
                *it->second.poly3_map_index,
                *it->second.mesh,
                pair.second.mask.get(),
                node_set.get());
        }
        pair.second.node_set = node_set;
        callbacks->project_run_checkpoint();
    }

    if (!p->volume_load_directives.empty()) {
        std::shared_ptr<ConcentratedLoad> total_cload(new ConcentratedLoad);
        for (const auto &pair : p->volume_load_directives) {
            for (auto it = p->element_directives.begin();
                    it != p->element_directives.end(); ++it) {
                load_volume(
                    *it->second.poly3_map,
                    *it->second.poly3_map_index,
                    *it->second.mesh,
                    pair.second.mask.get(),
                    pair.second.force_density,
                    total_cload.get());
            }
            break;
        }
        p->total_cload = total_cload;
        callbacks->project_run_checkpoint();
    }

    write_calculix_job(p->temp_dir, "main", *p);
    run_calculix(p->temp_dir, "main");
    Results results;
    std::ifstream frd_stream(p->temp_dir + "/main.frd");
    read_calculix_frd(
        frd_stream,
        NodeId::from_int(1),
        p->mesh_id_allocator.next_node_id,
        &results);
    p->results.reset(new Results(std::move(results)));
    callbacks->project_run_checkpoint();
}

} /* namespace os2cx */

