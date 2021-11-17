#include "gui_opengl_poly3.hpp"

namespace os2cx {

std::shared_ptr<const GuiOpenglScene> gui_opengl_scene_poly3(
    const Project &project,
    const GuiOpenglPoly3Callback *callback
) {
    GuiOpenglScene scene;
    for (const auto &pair : project.mesh_objects) {
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

            QColor color;
            callback->calculate_attributes(pair.first, sid, &color);

            QColor colors[3] = {color, color, color};

            for (const Plc3::Surface::Triangle &tri : surface.triangles) {
                Point ps[3];
                for (int i = 0; i < 3; ++i) {
                    ps[i] = plc->vertices[tri.vertices[i]].point;
                }
                if (outside_volume_index == 1) {
                    std::swap(ps[2], ps[1]);
                }
                Vector ds[3] = {Vector::zero(), Vector::zero(), Vector::zero()};
                scene.add_triangle(ps, ds, colors);
            }
        }
    }
    return std::make_shared<const GuiOpenglScene>(std::move(scene));
}

} /* namespace os2cx */
