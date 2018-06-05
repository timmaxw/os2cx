#include "project.hpp"

#include <fstream>

#include "calculix_frd_read.hpp"
#include "calculix_inp_write.hpp"
#include "calculix_run.hpp"
#include "mesher_tetgen.hpp"
#include "openscad_extract.hpp"

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
    for (auto &pair : p->select_surface_objects) {
        pair.second.mask = openscad_extract_poly3(
            p, "select_surface", pair.first);
        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->mesh_objects) {
        /* volume_mask and volume_mask_volumes are two parallel vectors.
        poly3_map_create will slice the mesh solid by the masks and then write
        out which volumes correspond to which masks into volume_mask_volumes,
        which ultimately points to VolumeObject::poly3_map_volumes. */
        std::vector<const Poly3 *> volume_masks;
        std::vector<std::set<Poly3Map::VolumeId> *> volume_mask_volumes;
        for (auto &select_volume_pair : p->select_volume_objects) {
            volume_masks.push_back(
                select_volume_pair.second.mask.get());
            volume_mask_volumes.push_back(
                &select_volume_pair.second.poly3_map_volumes[pair.first]);
        }

        /* Same with surface_mask and surface_mask_volumes. */
        std::vector<const Poly3 *> surface_masks;
        std::vector<std::set<Poly3Map::SurfaceId> *> surface_mask_surfaces;
        for (auto &select_surface_pair : p->select_surface_objects) {
            surface_masks.push_back(
                select_surface_pair.second.mask.get());
            surface_mask_volumes.push_back(
                &select_surface_pair.second.poly3_map_surfaces[pair.first]);
        }

        std::shared_ptr<Poly3Map> poly3_map(new Poly3Map);
        poly3_map_create(
            *pair.second.solid,
            volume_masks,
            surface_masks,
            poly3_map.get(),
            volume_mask_volumes,
            surface_mask_surfaces
        );
        pair.second.poly3_map = poly3_map;

        pair.second.poly3_map_index.reset(new Poly3MapIndex(*poly3_map));

        for (auto &mesh_pair : p->mesh_objects) {
            if (mesh_pair.first == pair.first) {
                /* This is the same MeshObject that we just constructed the
                poly3_map for; obviously it contains all its own Poly3Map
                volumes */
                std::set<Poly3Map::VolumeId> poly3_map_volumes;
                for (Poly3Map::VolumeId volume_id = 0;
                        volume_id < static_cast<int>(poly3_map->volumes.size());
                        ++volume_id) {
                    poly3_map_volumes.insert(volume_id);
                }
                mesh_pair.second.poly3_map_volumes[pair.first] =
                    poly3_map_volumes;
            } else {
                /* Other MeshObjects don't contain any volumes from this
                MeshObject's Poly3Map. */
                mesh_pair.second.poly3_map_volumes[pair.first] =
                    std::set<Poly3Map::VolumeId>();
            }
        }

        callbacks->project_run_checkpoint();
    }

    for (auto &pair : p->mesh_objects) {
        p->approx_scale = std::max(
            p->approx_scale,
            pair.second.poly3_map_index->approx_scale());
    }
    callbacks->project_run_checkpoint();

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
            ElementSet partial_element_set = compute_element_set_from_mask(
                *mesh_pair.second.poly3_map_index,
                *p->mesh,
                mesh_pair.second.element_begin,
                mesh_pair.second.element_end,
                pair.second.poly3_map_volumes.at(mesh_pair.first));
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

