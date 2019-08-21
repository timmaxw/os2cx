#ifndef OS2CX_PROJECT_HPP_
#define OS2CX_PROJECT_HPP_

#include <map>
#include <string>

#include "compute_attrs.hpp"
#include "mesh.hpp"
#include "mesh_index.hpp"
#include "plc.hpp"
#include "plc_nef.hpp"
#include "plc_index.hpp"
#include "result.hpp"
#include "units.hpp"

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
        scad_path(scad_path_),
        temp_dir(temp_dir_),
        progress(Progress::NothingDone),
        errored(false),
        next_bit_index(attr_bit_solid() + 1),
        approx_scale(Length(0))
        { }

    std::string scad_path;
    std::string temp_dir;

    Progress progress;
    bool errored;

    std::vector<std::string> inventory_errors;

    UnitSystem unit_system;

    std::vector<std::string> calculix_deck;

    AttrBitIndex next_bit_index;

    typedef std::string VolumeObjectName;
    typedef std::string SurfaceObjectName;
    typedef std::string LoadObjectName;
    typedef std::string MeshObjectName;
    typedef std::string SliceObjectName;
    typedef std::string SelectVolumeObjectName;
    typedef std::string SelectSurfaceObjectName;
    typedef std::string LoadVolumeObjectName;
    typedef std::string LoadSurfaceObjectName;
    typedef std::string MaterialObjectName;

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

    class LoadObject {
    public:
        std::shared_ptr<const ConcentratedLoad> load;
    };

    class MeshObject : public VolumeObject {
    public:
        enum class Mesher { Tetgen, NaiveBricks };
        Mesher mesher;

        /* If max_element_size is set to the magic value
        SUGGEST_MAX_ELEMENT_SIZE, then we'll automatically choose a reasonable
        value for max_element_size by examining the PLC. */
        static constexpr double SUGGEST_MAX_ELEMENT_SIZE = -1;
        double max_element_size;

        std::shared_ptr<const Poly3> solid;
        std::shared_ptr<const Plc3> plc;
        std::shared_ptr<const Plc3Index> plc_index;

        /* The partial_meshes of all the individual MeshObjects will be combined
        to form the overall project mesh. The nodes and elements will be
        assigned new IDs when this happens, so partial_mesh shouldn't be used
        for anything on its own. In addition, it will be nulled after it's been
        combined into the overall project mesh. */
        std::shared_ptr<const Mesh3> partial_mesh;

        /* Each SliceObject is applied separately to each MeshObject, and the
        results are stored temporarily in partial_slices. Later, all the Slices
        from different MeshObjects for the same SliceObject are combined into
        the top-level SliceObject's Slice. The nodes will be assigned new IDs
        when the meshes are combined, so partial_slices shouldn't be used for
        anything on its own. In addition, the partial_slices map will be cleared
        after the Slices have been combined. */
        std::map<SliceObjectName, std::shared_ptr<const Slice> > partial_slices;

        /* Once the partial meshes have been combined, we record here the ranges
        of node and element IDs in the combined mesh that corresponded to this
        mesh object. */
        NodeId node_begin, node_end;
        ElementId element_begin, element_end;
    };

    std::map<MeshObjectName, MeshObject> mesh_objects;

    class SliceObject {
    public:
        AttrBitIndex bit_index;
        std::shared_ptr<const Poly3> mask;
        Vector direction_vector;
        double direction_angle_tolerance;
        std::shared_ptr<const Slice> slice;
        std::shared_ptr<const std::vector<LinearEquation> > equations;
    };

    std::map<SliceObjectName, SliceObject> slice_objects;

    class SelectVolumeObject : public VolumeObject {
    public:
        AttrBitIndex bit_index;
        std::shared_ptr<const Poly3> mask;
    };

    std::map<SelectVolumeObjectName, SelectVolumeObject> select_volume_objects;

    class SelectSurfaceObject : public SurfaceObject {
    public:
        AttrBitIndex bit_index;
        std::shared_ptr<const Poly3> mask;
        enum class Mode { External, Internal } mode;
        Vector direction_vector;
        double direction_angle_tolerance;
    };

    std::map<SelectSurfaceObjectName, SelectSurfaceObject>
        select_surface_objects;

    /* This mesh is formed by combining the meshes of all the individual mesh
    objects. */
    std::shared_ptr<const Mesh3> mesh;
    std::shared_ptr<const Mesh3Index> mesh_index;

    class LoadVolumeObject : public LoadObject {
    public:
        VolumeObjectName volume;
        WithUnit<Vector> force_total_or_per_volume;
        bool force_is_per_volume;
    };

    std::map<LoadVolumeObjectName, LoadVolumeObject> load_volume_objects;

    class LoadSurfaceObject : public LoadObject {
    public:
        SurfaceObjectName surface;
        WithUnit<Vector> force_total_or_per_area;
        bool force_is_per_area;
    };

    std::map<LoadSurfaceObjectName, LoadSurfaceObject> load_surface_objects;

    class MaterialObject {
    public:
        WithUnit<double> youngs_modulus;
        double poissons_ratio;
    };

    std::map<MaterialObjectName, MaterialObject> material_objects;

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

} /* namespace os2cx */

#endif
