#include "gui_opengl_poly3.hpp"

namespace os2cx {

void gui_opengl_scene_poly3_vertex(
    const GuiOpenglPoly3Callback *callback,
    const std::string &node_object_name,
    const Project::NodeObject &node_object,
    GuiOpenglScene *scene
) {
    QColor color;
    bool xray = false;
    callback->calculate_vertex_attributes(
        node_object_name, &color, &xray);
    if (color.isValid()) {
        scene->add_vertex(
            node_object.point, ComplexVector::zero(), color, xray);
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

            QColor color;
            bool xray = false;
            callback->calculate_surface_attributes(
                pair.first, sid, &color, &xray);

            int outside_volume_index;
            if (!xray) {
                if (surface.volumes[0] == plc->volume_outside) {
                    outside_volume_index = 0;
                } else if (surface.volumes[1] == plc->volume_outside) {
                    outside_volume_index = 1;
                } else {
                    /* Surface is hidden; don't draw it. */
                    continue;
                }
            }

            QColor colors[3] = {color, color, color};

            for (const Plc3::Surface::Triangle &tri : surface.triangles) {
                Point ps[3];
                ComplexVector ds[3];
                for (int i = 0; i < 3; ++i) {
                    ps[i] = plc->vertices[tri.vertices[i]].point;
                    ds[i] = ComplexVector::zero();
                }
                if (xray) {
                    scene.add_triangle(ps, ds, colors, true);
                } else {
                    if (outside_volume_index == 1) {
                        /* In non-xray mode, the orientation is used for
                        lighting and/or backface culling, so we need to ensure
                        the triangle is facing the right way */
                        std::swap(ps[2], ps[1]);
                    }
                    scene.add_triangle(ps, ds, colors, false);
                }

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
