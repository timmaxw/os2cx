#include "gui_opengl_poly3.hpp"

namespace os2cx {

void gui_opengl_scene_poly3_vertex(
    const GuiOpenglPoly3Callback *callback,
    const std::string &node_object_name,
    const Project::NodeObject &node_object,
    GuiOpenglScene *scene
) {
    QColor vertex_color;
    bool xray = false;
    callback->calculate_vertex_attributes(
        node_object_name, &vertex_color, &xray);
    if (vertex_color.isValid()) {
        scene->add_vertex(
            node_object.point, ComplexVector::zero(), vertex_color, xray);
    }
}

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
            callback->calculate_surface_attributes(
                pair.first, sid, &color);

            QColor colors[3] = {color, color, color};

            for (const Plc3::Surface::Triangle &tri : surface.triangles) {
                Point ps[3];
                ComplexVector ds[3];
                for (int i = 0; i < 3; ++i) {
                    ps[i] = plc->vertices[tri.vertices[i]].point;
                    ds[i] = ComplexVector::zero();
                }
                if (outside_volume_index == 1) {
                    std::swap(ps[2], ps[1]);
                }
                scene.add_triangle(ps, ds, colors);
            }
        }
    }

    for (const auto &pair : project.select_node_objects) {
        gui_opengl_scene_poly3_vertex(
            callback, pair.first, pair.second, &scene);
    }
    for (const auto &pair : project.create_node_objects) {
        gui_opengl_scene_poly3_vertex(
            callback, pair.first, pair.second, &scene);
    }

    return std::make_shared<const GuiOpenglScene>(std::move(scene));
}

} /* namespace os2cx */
