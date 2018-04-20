#ifndef OS2CX_PROJECT_HPP_
#define OS2CX_PROJECT_HPP_

#include <map>
#include <string>

#include "attrs.hpp"
#include "mesh.hpp"
#include "mesh_index.hpp"
#include "region.hpp"
#include "region_map.hpp"
#include "region_map_index.hpp"
#include "result.hpp"

namespace os2cx {

class Project {
public:
    Project(const std::string &scad_path_, const std::string &temp_dir_) :
        scad_path(scad_path_), temp_dir(temp_dir_) { }

    std::string scad_path;
    std::string temp_dir;

    std::vector<std::string> inventory_errors;

    std::vector<std::string> directives;

    MeshIdAllocator mesh_id_allocator;

    class ElementDirective {
    public:
        std::unique_ptr<Region3> solid;
        std::unique_ptr<RegionMap3> region_map;
        std::unique_ptr<RegionMap3Index> region_map_index;
        std::unique_ptr<Mesh3> mesh;
        std::unique_ptr<Mesh3Index> mesh_index;
    };
    std::map<std::string, ElementDirective> element_directives;

    class NSetDirective {
    public:
        std::unique_ptr<Region3> mask;
        std::unique_ptr<NodeSet> node_set;
    };
    std::map<std::string, NSetDirective> nset_directives;

    class VolumeLoadDirective {
    public:
        VolumeLoadDirective() : added_to_total_cload(false) { }
        std::unique_ptr<Region3> mask;
        ForceDensityVector force_density;
        bool added_to_total_cload;
    };
    std::map<std::string, VolumeLoadDirective> volume_load_directives;

    std::unique_ptr<ConcentratedLoad> total_cload;

    std::unique_ptr<Results> results;
};

class ProjectTask {
public:
    enum class Type {
        ExtractInventory, // no target
        ExtractElement, // target = element directive
        ExtractNset, // target = nset directive
        ExtractVolumeLoad, // target = volume load directive
        Map, // target = element directive
        Mesh, // target = element directive
        ComputeNset, // target = nset directive
        ComputeVolumeLoad, // target = volume load directive
        Calculix // no target
    };

    bool operator==(const ProjectTask &other) const {
        return type == other.type && target == other.target;
    }

    bool is_done(const Project *project) const;
    static bool next_task(const Project *project, ProjectTask *out);
    void execute(Project *project) const;

    Type type;
    std::string target;
};

} /* namespace os2cx */

#endif
