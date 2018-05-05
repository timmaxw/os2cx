#ifndef OS2CX_PROJECT_HPP_
#define OS2CX_PROJECT_HPP_

#include <map>
#include <string>

#include "attrs.hpp"
#include "mesh.hpp"
#include "mesh_index.hpp"
#include "poly.hpp"
#include "poly_map.hpp"
#include "poly_map_index.hpp"
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
        std::shared_ptr<const Poly3> solid;
        std::shared_ptr<const Poly3Map> poly3_map;
        std::shared_ptr<const Poly3MapIndex> poly3_map_index;
        std::shared_ptr<const Mesh3> mesh;
        std::shared_ptr<const Mesh3Index> mesh_index;
    };
    std::map<std::string, ElementDirective> element_directives;

    class NSetDirective {
    public:
        std::shared_ptr<const Poly3> mask;
        std::shared_ptr<const NodeSet> node_set;
    };
    std::map<std::string, NSetDirective> nset_directives;

    class VolumeLoadDirective {
    public:
        std::shared_ptr<const Poly3> mask;
        ForceDensityVector force_density;
    };
    std::map<std::string, VolumeLoadDirective> volume_load_directives;

    std::shared_ptr<const ConcentratedLoad> total_cload;

    std::shared_ptr<const Results> results;
};

/* project_run() performs all of the computations for a project. It gets run in
a separate worker thread separate from the main application thread. Each thread
has its own copy of the Project; project_run() periodically calls callbacks->
project_run_checkpoint(), which synchronizes with the main application thread to
copy the latest project state over from the worker thread. The copying process
is inexpensive because all the complex data structures on the Project are stored
as shared_ptr<const Whatever>. */

class ProjectRunCallbacks {
public:
    virtual void project_run_checkpoint() = 0;
};

void project_run(Project *project, ProjectRunCallbacks *callbacks);

} /* namespace os2cx */

#endif
