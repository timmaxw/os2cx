#include "gui_opengl_poly3.hpp"

namespace os2cx {

void gui_opengl_scene_poly3_vertex(
    const GuiOpenglPoly3Callback *callback,
    const std::string &node_object_name,
    const Project::NodeObject &node_object,
    bool xray,
    GuiOpenglScene *scene
) {
    QColor color;
    callback->calculate_vertex_attributes(node_object_name, &color);
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

    std::set<std::pair<std::string, Plc3::SurfaceId> > xray_surfaces;
    std::set<std::string> xray_node_object_names;
    callback->calculate_xrays(&xray_surfaces, &xray_node_object_names);

    for (const auto &pair : project.mesh_objects) {
        const Plc3 *plc = pair.second.plc.get();
        if (plc == nullptr) {
            continue;
        }

        for (Plc3::SurfaceId sid = 0;
                sid < static_cast<int>(plc->surfaces.size()); ++sid) {
            const Plc3::Surface &surface = plc->surfaces[sid];

            bool xray = xray_surfaces.count(std::make_pair(pair.first, sid));
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
        bool xray = xray_node_object_names.count(pair.first);
        gui_opengl_scene_poly3_vertex(
            callback, pair.first, pair.second, xray, &scene);
    }
    for (const auto &pair : project.create_node_objects) {
        bool xray = xray_node_object_names.count(pair.first);
        gui_opengl_scene_poly3_vertex(
            callback, pair.first, pair.second, xray, &scene);
    }

    return std::make_shared<const GuiOpenglScene>(std::move(scene));
}

} /* namespace os2cx */
