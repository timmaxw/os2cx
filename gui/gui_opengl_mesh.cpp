#include "gui_opengl_mesh.hpp"

namespace os2cx {

std::shared_ptr<const GuiOpenglScene> gui_opengl_scene_mesh(
    const Project &project,
    const GuiOpenglMeshCallback *callback
) {
    GuiOpenglScene scene;
    for (const FaceId &fi : project.mesh_index->unmatched_faces) {
        const Element3 &element = project.mesh->elements[fi.element_id];
        const ElementTypeInfo &type = ElementTypeInfo::get(element.type);
        NodeId node_ids[ElementShapeInfo::max_vertices_per_face];
        const ElementShapeInfo::Face &face = type.shape->faces[fi.face];
        for (int i = 0; i < static_cast<int>(face.vertices.size()); ++i) {
            node_ids[i] = element.nodes[face.vertices[i]];
        }

        Point ps[ElementShapeInfo::max_vertices_per_face];
        QColor cs[ElementShapeInfo::max_vertices_per_face];
        for (int i = 0; i < static_cast<int>(face.vertices.size()); ++i) {
            ps[i] = project.mesh->nodes[node_ids[i]].point;
            Vector disp;
            callback->calculate_attributes(
                fi.element_id, fi.face, node_ids[i], &cs[i], &disp);
            ps[i] += disp;
        }

        if (face.vertices.size() == 3) {
            scene.add_triangle(ps, cs);
        } else if (face.vertices.size() == 6) {
            int ixs[4][3] = {{0, 1, 5}, {2, 3, 1}, {4, 5, 3}, {1, 3, 5}};
            for (int i = 0; i < 4; ++i) {
                Point subps[3] = {ps[ixs[i][0]], ps[ixs[i][1]], ps[ixs[i][2]]};
                QColor subcs[3] = {cs[ixs[i][0]], cs[ixs[i][1]], cs[ixs[i][2]]};
                scene.add_triangle(subps, subcs);
            }
        } else {
            assert(false);
        }

        for (int i = 0; i < static_cast<int>(face.vertices.size()); ++i) {
            int j = (i + 1) % face.vertices.size();
            /* Every line is part of two triangles, but the two triangles
            traverse it in opposite directions. Only draw one of the two
            directions so that we don't draw each line twice. */
            if (node_ids[i].to_int() > node_ids[j].to_int()) {
                Point line_ps[2];
                line_ps[0] = ps[i];
                line_ps[1] = ps[j];
                scene.add_line(line_ps);
            }
        }
    }
    return std::make_shared<const GuiOpenglScene>(std::move(scene));
}

} /* namespace os2cx */
