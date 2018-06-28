#ifndef OS2CX_PROJECT_HPP_
#define OS2CX_PROJECT_HPP_

#include <map>
#include <string>

#include "attrs.hpp"
#include "mesh.hpp"
#include "mesh_index.hpp"
#include "plc.hpp"
#include "plc_nef.hpp"
#include "plc_index.hpp"
#include "result.hpp"

namespace os2cx {

class Project {
public:
    enum class Progress {
        NothingDone,
        InventoryDone,
        PolysDone,
        PolyAttrsDone,
        MeshDone,
        MeshAttrsDone,
        ResultsDone,
        AllDone = ResultsDone
    };

    Project(const std::string &scad_path_, const std::string &temp_dir_) :
        progress(Progress::NothingDone),
        scad_path(scad_path_),
        temp_dir(temp_dir_),
        next_bit_index(bit_index_solid() + 1),
        approx_scale(Length(0))
        { }

    Progress progress;

    std::string scad_path;
    std::string temp_dir;

    std::vector<std::string> inventory_errors;

    std::vector<std::string> calculix_deck;

    Plc3::BitIndex next_bit_index;

    typedef std::string VolumeObjectName;
    typedef std::string SurfaceObjectName;
    typedef std::string MeshObjectName;
    typedef std::string SelectVolumeObjectName;
    typedef std::string SelectSurfaceObjectName;
    typedef std::string LoadObjectName;

    class VolumeObject {
    public:
        std::shared_ptr<const ElementSet> element_set;
        std::shared_ptr<const NodeSet> node_set;
    };

    class SurfaceObject {
    public:
        std::shared_ptr<const FaceSet> face_set;
        std::shared_ptr<const NodeSet> node_set;
    };

    class MeshObject : public VolumeObject {
    public:
        std::shared_ptr<const Poly3> solid;
        std::shared_ptr<const Plc3> plc;
        std::shared_ptr<const Plc3Index> plc_index;

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

    std::map<MeshObjectName, MeshObject> mesh_objects;

    class SelectVolumeObject : public VolumeObject {
    public:
        Plc3::BitIndex bit_index;
        std::shared_ptr<const Poly3> mask;
    };

    std::map<SelectVolumeObjectName, SelectVolumeObject> select_volume_objects;

    class SelectSurfaceObject : public SurfaceObject {
    public:
        Plc3::BitIndex bit_index;
        std::shared_ptr<const Poly3> mask;
        Vector direction_vector;
        double direction_angle_tolerance;
    };

    std::map<SelectSurfaceObjectName, SelectSurfaceObject>
        select_surface_objects;

    /* This mesh is formed by combining the meshes of all the individual mesh
    objects. */
    std::shared_ptr<const Mesh3> mesh;
    std::shared_ptr<const Mesh3Index> mesh_index;

    class LoadObject {
    public:
        VolumeObjectName volume;
        Vector force_density;
        std::shared_ptr<const ConcentratedLoad> load;
    };

    std::map<LoadObjectName, LoadObject> load_objects;

    std::shared_ptr<const Results> results;

    /* A very rough/informal approximation of the project's typical length
    scale. Will be zero until all the Poly3s have been loaded. */
    Length approx_scale;

    const VolumeObject *find_volume_object(const VolumeObjectName &name) const {
        auto it = mesh_objects.find(name);
        if (it != mesh_objects.end()) {
            return &it->second;
        }
        auto jt = select_volume_objects.find(name);
        if (jt != select_volume_objects.end()) {
            return &jt->second;
        }
        return nullptr;
    }

    const SurfaceObject *find_surface_object(
            const SurfaceObjectName &name) const {
        auto jt = select_surface_objects.find(name);
        if (jt != select_surface_objects.end()) {
            return &jt->second;
        }
        return nullptr;
    }
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
    virtual void project_run_checkpoint() { }
};

void project_run(Project *project, ProjectRunCallbacks *callbacks);

} /* namespace os2cx */

#endif
