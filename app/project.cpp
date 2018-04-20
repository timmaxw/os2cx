#include "project.hpp"

#include <fstream>

#include "calculix_frd_read.hpp"
#include "calculix_inp_write.hpp"
#include "calculix_run.hpp"
#include "mesher_tetgen.hpp"
#include "openscad_extract.hpp"

namespace os2cx {

bool ProjectTask::is_done(const Project *p) const {
    switch (type) {
    case Type::ExtractInventory:
        return !p->inventory_errors.empty()
            || !p->element_directives.empty();
    case Type::ExtractElement:
        return p->element_directives.find(target)->second.solid != nullptr;
    case Type::ExtractNset:
        return p->nset_directives.find(target)->second.mask != nullptr;
    case Type::ExtractVolumeLoad:
        return p->volume_load_directives.find(target)->second.mask != nullptr;
    case Type::Map:
        return p->element_directives.find(target)->second.region_map != nullptr;
    case Type::Mesh:
        return p->element_directives.find(target)->second.mesh != nullptr;
    case Type::ComputeNset:
        return p->nset_directives.find(target)->second.node_set != nullptr;
    case Type::ComputeVolumeLoad:
        return p->volume_load_directives.find(target)->second
            .added_to_total_cload;
    default:
        assert(false);
    }
}

bool ProjectTask::next_task(const Project *p, ProjectTask *task) {
    if (p->inventory_errors.empty() &&
            p->element_directives.empty()) {
        task->type = Type::ExtractInventory;
        return true;
    }

    for (const auto &pair : p->element_directives) {
        if (!pair.second.solid) {
            task->type = Type::ExtractElement;
            task->target = pair.first;
            return true;
        }
    }

    for (const auto &pair : p->nset_directives) {
        if (!pair.second.mask) {
            task->type = Type::ExtractNset;
            task->target = pair.first;
            return true;
        }
    }

    for (const auto &pair : p->volume_load_directives) {
        if (!pair.second.mask) {
            task->type = Type::ExtractVolumeLoad;
            task->target = pair.first;
            return true;
        }
    }

    for (const auto &pair : p->element_directives) {
        if (!pair.second.region_map) {
            task->type = Type::Map;
            task->target = pair.first;
            return true;
        }
    }

    for (const auto &pair : p->element_directives) {
        if (pair.second.region_map && !pair.second.mesh) {
            task->type = Type::Mesh;
            task->target = pair.first;
            return true;
        }
    }

    for (const auto &pair : p->nset_directives) {
        if (!pair.second.node_set) {
            task->type = Type::ComputeNset;
            task->target = pair.first;
            return true;
        }
    }

    for (const auto &pair : p->volume_load_directives) {
        if (!pair.second.added_to_total_cload) {
            task->type = Type::ComputeVolumeLoad;
            task->target = pair.first;
            return true;
        }
    }

    if (!p->results) {
        task->type = Type::Calculix;
        return true;
    }

    return false;
}

std::vector<const Region3 *> get_volume_masks(const Project *project) {
    std::vector<const Region3 *> volume_masks;
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

void ProjectTask::execute(Project *p) const {
    switch (type) {
    case Type::ExtractInventory:
        maybe_create_directory(p->temp_dir);
        openscad_extract_inventory(p);
        break;
    case Type::ExtractElement:
        p->element_directives[target].solid = openscad_extract_region(
            p, "element", target);
        break;
    case Type::ExtractNset:
        p->nset_directives[target].mask = openscad_extract_region(
            p, "nset", target);
        break;
    case Type::ExtractVolumeLoad:
        p->volume_load_directives[target].mask = openscad_extract_region(
            p, "volume_load", target);
        break;
    case Type::Map: {
        Project::ElementDirective &ed = p->element_directives[target];
        ed.region_map.reset(new RegionMap3);
        region_map_create(
            *ed.solid,
            get_volume_masks(p),
            ed.region_map.get());
        ed.region_map_index.reset(new RegionMap3Index(*ed.region_map));
        break;
    }
    case Type::Mesh: {
        Project::ElementDirective &ed = p->element_directives[target];
        Mesh3 mesh = mesher_tetgen(*ed.region_map);
        ed.mesh.reset(new Mesh3(
            p->mesh_id_allocator.allocate(std::move(mesh))));
        ed.mesh_index.reset(new Mesh3Index(ed.mesh.get()));
        break;
    }
    case Type::ComputeNset: {
        Project::NSetDirective &nd = p->nset_directives[target];
        nd.node_set.reset(new NodeSet);
        for (auto it = p->element_directives.begin();
                it != p->element_directives.end(); ++it) {
            node_set_volume(
                *it->second.region_map,
                *it->second.region_map_index,
                *it->second.mesh,
                nd.mask.get(),
                nd.node_set.get());
        }
        break;
    }
    case Type::ComputeVolumeLoad: {
        if (!p->total_cload) {
            p->total_cload.reset(new ConcentratedLoad);
        }
        Project::VolumeLoadDirective &vld = p->volume_load_directives[target];
        for (auto it = p->element_directives.begin();
                it != p->element_directives.end(); ++it) {
            load_volume(
                *it->second.region_map,
                *it->second.region_map_index,
                *it->second.mesh,
                vld.mask.get(),
                vld.force_density,
                p->total_cload.get());
        }
        vld.added_to_total_cload = true;
        break;
    }
    case Type::Calculix: {
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
        break;
    }
    default:
        assert(false);
    }
}

} /* namespace os2cx */

