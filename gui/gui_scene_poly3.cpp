#include "gui_scene_poly3.hpp"

namespace os2cx {

GuiScenePoly3::GuiScenePoly3(const SceneParams &params) :
    GuiSceneAbstract(params) { }

void GuiScenePoly3::initialize_scene() {
    for (const auto &pair : project->mesh_objects) {
        const Plc3 *plc = pair.second.plc.get();
        if (plc == nullptr) {
            continue;
        }
        for (Plc3::SurfaceId sid = 0;
                sid < static_cast<int>(plc->surfaces.size()); ++sid) {
            const Plc3::Surface &surface = plc->surfaces[sid];

            int outside_volume_index;
            if (surface.volumes[0] == plc->volume_outside) {
                outside_volume_index = 0;
            } else if (surface.volumes[1] == plc->volume_outside) {
                outside_volume_index = 1;
            } else {
                continue;
            }
            Plc3::VolumeId volume_id =
                surface.volumes[1 - outside_volume_index];

            QColor color = surface_color(pair.first, volume_id);
            QColor colors[3] = {color, color, color};

            for (const Plc3::Surface::Triangle &tri : surface.triangles) {
                Point ps[3];
                for (int i = 0; i < 3; ++i) {
                    ps[i] = plc->vertices[tri.vertices[i]].point;
                }
                if (outside_volume_index == 1) {
                    std::swap(ps[2], ps[1]);
                }
                add_triangle(ps, colors);
            }
        }
    }
}

QColor GuiScenePoly3::surface_color(
    const Project::MeshObjectName &mesh_object_name,
    Plc3::VolumeId volume_id
) {
    (void)mesh_object_name;
    (void)volume_id;
    return QColor(0x80, 0x80, 0x80);
}

GuiScenePoly3Mesh::GuiScenePoly3Mesh(
    const SceneParams &params,
    const Project::MeshObjectName &mesh_
) :
    GuiScenePoly3(params),
    mesh(mesh_)
{ }

QColor GuiScenePoly3Mesh::surface_color(
    const Project::MeshObjectName &mesh_object_name,
    Plc3::VolumeId volume_id
) {
    (void)volume_id;
    if (mesh_object_name == mesh) {
        return QColor(0xFF, 0x00, 0x00);
    } else {
        return QColor(0x80, 0x80, 0x80);
    }
}

GuiScenePoly3SelectVolume::GuiScenePoly3SelectVolume(
    const SceneParams &params,
    const Project::SelectVolumeObjectName &select_volume
) :
    GuiScenePoly3(params),
    bit_index(project->select_volume_objects.at(select_volume).bit_index)
{ }

QColor GuiScenePoly3SelectVolume::surface_color(
    const Project::MeshObjectName &mesh_object_name,
    Plc3::VolumeId volume_id
) {
    const Plc3 *plc = project->mesh_objects.at(mesh_object_name).plc.get();
    if (plc->volumes[volume_id].bitset[bit_index]) {
        return QColor(0xFF, 0x00, 0x00);
    } else {
        return QColor(0x80, 0x80, 0x80);
    }
}

} /* namespace os2cx */
