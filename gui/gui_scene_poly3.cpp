#include "gui_scene_poly3.hpp"

namespace os2cx {

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

            QColor color(0x80, 0x80, 0x80);
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

} /* namespace os2cx */
