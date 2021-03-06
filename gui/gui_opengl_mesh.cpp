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

        int n;
        const int (*ixs)[3];
        if (face.vertices.size() == 4) {
            static const int rect_4_ixs[2][3] = {{0, 1, 2}, {0, 2, 3}};
            n = 2;
            ixs = rect_4_ixs;
        } else if (face.vertices.size() == 8) {
            static const int rect_8_ixs[6][3] = {
                {0, 1, 7}, {1, 2, 3}, {3, 4, 5}, {5, 6, 7}, {1, 3, 5}, {1, 5, 7}
            };
            n = 6;
            ixs = rect_8_ixs;
        } else if (face.vertices.size() == 3) {
            static const int tri_3_ixs[1][3] = {{0, 1, 2}};
            n = 1;
            ixs = tri_3_ixs;
        } else if (face.vertices.size() == 6) {
            static const int tri_6_ixs[4][3] = {
                {0, 1, 5}, {2, 3, 1}, {4, 5, 3}, {1, 3, 5}
            };
            n = 4;
            ixs = tri_6_ixs;
        } else {
            assert(false);
        }
        for (int i = 0; i < n; ++i) {
            Point subps[3] = {ps[ixs[i][0]], ps[ixs[i][1]], ps[ixs[i][2]]};
            QColor subcs[3] = {cs[ixs[i][0]], cs[ixs[i][1]], cs[ixs[i][2]]};
            scene.add_triangle(subps, subcs);
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
