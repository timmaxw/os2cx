#include "gui_scene_poly3.hpp"

namespace os2cx {

GuiScenePoly3::GuiScenePoly3(const SceneParams &params) :
    GuiSceneAbstract(params) { }

void GuiScenePoly3::initialize_scene() {
    for (const auto &pair : project->mesh_objects) {
        const Poly3Map *poly3_map = pair.second.poly3_map.get();
        if (poly3_map == nullptr) {
            continue;
        }
        for (Poly3Map::SurfaceId sid = 0;
                sid < static_cast<int>(poly3_map->surfaces.size()); ++sid) {
            const Poly3Map::Surface &surface = poly3_map->surfaces[sid];

            int outside_volume_index;
            if (surface.volumes[0] == poly3_map->volume_outside) {
                outside_volume_index = 0;
            } else if (surface.volumes[1] == poly3_map->volume_outside) {
                outside_volume_index = 1;
            } else {
                continue;
            }
            Poly3Map::VolumeId volume_id =
                surface.volumes[1 - outside_volume_index];

            QColor color = surface_color(pair.first, volume_id);
            QColor colors[3] = {color, color, color};

            for (const Poly3Map::Surface::Triangle &tri : surface.triangles) {
                Point ps[3];
                for (int i = 0; i < 3; ++i) {
                    ps[i] = poly3_map->vertices[tri.vertices[i]].point;
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
    Poly3Map::VolumeId volume_id
) {
    (void)mesh_object_name;
    (void)volume_id;
    return QColor(0x80, 0x80, 0x80);
}

GuiScenePoly3Volume::GuiScenePoly3Volume(
    const SceneParams &params,
    const Project::VolumeObjectName &volume
) :
    GuiScenePoly3(params),
    volume(volume)
{ }

QColor GuiScenePoly3Volume::surface_color(
    const Project::MeshObjectName &mesh_object_name,
    Poly3Map::VolumeId volume_id
) {
    const Project::VolumeObject *volume_obj =
        project->find_volume_object(volume);
    if (volume_obj->poly3_map_volumes.at(mesh_object_name).count(volume_id)) {
        return QColor(0xFF, 0x00, 0x00);
    } else {
        return QColor(0x80, 0x80, 0x80);
    }
}

} /* namespace os2cx */
