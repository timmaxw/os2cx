#include "gui_opengl_mesh.hpp"

namespace os2cx {

std::shared_ptr<const GuiOpenglScene> gui_opengl_scene_mesh(
    const Project &project,
    const GuiOpenglMeshCallback *callback
) {
    GuiOpenglScene scene;
    for (const FaceId &fi : project.mesh_index->unmatched_faces) {
        const Element3 &element = project.mesh->elements[fi.element_id];
        const ElementTypeShape &shape = element_type_shape(element.type);
        NodeId node_ids[ElementTypeShape::max_vertices_per_face];
        const ElementTypeShape::Face &face = shape.faces[fi.face];
        for (int i = 0; i < static_cast<int>(face.vertices.size()); ++i) {
            node_ids[i] = element.nodes[face.vertices[i]];
        }

        Point ps[ElementTypeShape::max_vertices_per_face];
        QColor cs[ElementTypeShape::max_vertices_per_face];
        for (int i = 0; i < static_cast<int>(face.vertices.size()); ++i) {
            ps[i] = project.mesh->nodes[node_ids[i]].point;
            Vector disp;
            callback->calculate_attributes(
                fi.element_id, fi.face, node_ids[i], &cs[i], &disp);
            ps[i] += disp;
        }

        if (face.vertices.size() == 3) {
            scene.add_triangle(ps, cs);
        } else if (face.vertices.size() == 4) {
            Point subps1[3] = {ps[0], ps[1], ps[2]};
            QColor subcs1[3] = {cs[0], cs[1], cs[2]};
            scene.add_triangle(subps1, subcs1);
            Point subps2[3] = {ps[0], ps[2], ps[3]};
            QColor subcs2[3] = {cs[0], cs[2], cs[3]};
            scene.add_triangle(subps2, subcs2);
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
