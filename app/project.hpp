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

    std::vector<std::string> calculix_deck;

    class MeshObject {
    public:
        std::shared_ptr<const Poly3> solid;
        std::shared_ptr<const Poly3Map> poly3_map;
        std::shared_ptr<const Poly3MapIndex> poly3_map_index;

        /* The partial_meshes of all the individual MeshObjects will be combined
        to form the overall project mesh. The nodes and elements will be
        assigned new IDs when this happens, so partial_mesh shouldn't be used
        for anything on its own. In addition, it will be nulled after it's been
        combined into the overall project mesh. */
        std::shared_ptr<const Mesh3> partial_mesh;

        /* Once the partial meshes have been combined, we record here the ranges
        of node and element IDs in the combined mesh that corresponded to this
        mesh object. */
        NodeId node_begin, node_end;
        ElementId element_begin, element_end;
    };
    std::map<std::string, MeshObject> mesh_objects;

    class SelectVolumeObject {
    public:
        std::shared_ptr<const Poly3> mask;
    };
    std::map<std::string, SelectVolumeObject> select_volume_objects;

    /* This mesh is formed by combining the meshes of all the individual mesh
    objects. */
    std::shared_ptr<const Mesh3> mesh;
    std::shared_ptr<const Mesh3Index> mesh_index;

    /* Every MeshObject and SelectVolumeObject has an associated VolumeObject
    with the same name. */
    class VolumeObject {
    public:
        std::map<std::string, std::set<Poly3Map::VolumeId> > poly3_map_volumes;
        std::shared_ptr<const ElementSet> element_set;
        std::shared_ptr<const NodeSet> node_set;
    };
    std::map<std::string, VolumeObject> volume_objects;

    class LoadObject {
    public:
        std::string volume;
        ForceDensityVector force_density;
        std::shared_ptr<const ConcentratedLoad> load;
    };
    std::map<std::string, LoadObject> load_objects;

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
