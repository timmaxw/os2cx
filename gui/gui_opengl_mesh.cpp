#include "gui_opengl_mesh.hpp"

namespace os2cx {

void gui_opengl_scene_mesh_face(
    const Project &project,
    const GuiOpenglMeshCallback *callback,
    FaceId face_id,
    bool xray,
    GuiOpenglScene *scene
) {
    const Element3 &element = project.mesh->elements[face_id.element_id];
    const ElementTypeShape &shape = element_type_shape(element.type);
    NodeId node_ids[ElementTypeShape::max_vertices_per_face];
    const ElementTypeShape::Face &face = shape.faces[face_id.face];
    for (int i = 0; i < static_cast<int>(face.vertices.size()); ++i) {
        node_ids[i] = element.nodes[face.vertices[i]];
    }

    Point ps[ElementTypeShape::max_vertices_per_face];
    ComplexVector ds[ElementTypeShape::max_vertices_per_face];
    QColor cs[ElementTypeShape::max_vertices_per_face];
    for (int i = 0; i < static_cast<int>(face.vertices.size()); ++i) {
        ps[i] = project.mesh->nodes[node_ids[i]].point;
        callback->calculate_face_attributes(
            face_id, node_ids[i], &ds[i], &cs[i]);
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
        Point subps[3] =
            {ps[ixs[i][0]], ps[ixs[i][1]], ps[ixs[i][2]]};
        ComplexVector subds[3] =
            {ds[ixs[i][0]], ds[ixs[i][1]], ds[ixs[i][2]]};
        QColor subcs[3] =
            {cs[ixs[i][0]], cs[ixs[i][1]], cs[ixs[i][2]]};
        scene->add_triangle(subps, subds, subcs, xray);
    }

    for (int i = 0; i < static_cast<int>(face.vertices.size()); ++i) {
        int j = (i + 1) % face.vertices.size();
        /* Every line is part of two triangles, but the two triangles
        traverse it in opposite directions. Only draw one of the two
        directions so that we don't draw each line twice. */
        if (node_ids[i].to_int() > node_ids[j].to_int()) {
            Point line_ps[2] = {ps[i], ps[j]};
            ComplexVector line_ds[2] = {ds[i], ds[j]};
            scene->add_line(line_ps, line_ds, xray);
        }
    }
}

void gui_opengl_scene_mesh_vertex(
    const Project &project,
    const GuiOpenglMeshCallback *callback,
    const std::string &node_object_name,
    const Project::NodeObject &node_object,
    bool xray,
    GuiOpenglScene *scene
) {
    NodeId node_id = node_object.node_id;
    const Node3 &node = project.mesh->nodes[node_id];
    QColor color;
    ComplexVector delta;
    callback->calculate_vertex_attributes(
        node_object_name, &delta, &color);
    if (color.isValid()) {
        scene->add_vertex(node.point, delta, color, xray);
    }
}

std::shared_ptr<const GuiOpenglScene> gui_opengl_scene_mesh(
    const Project &project,
    const GuiOpenglMeshCallback *callback,
    const GuiOpenglScene::AnimateMode animate_mode,
    double animate_hz
) {
    GuiOpenglScene scene;

    FaceSet xray_faces;
    std::set<std::string> xray_node_object_names;
    callback->calculate_xrays(&xray_faces, &xray_node_object_names);

    for (FaceId face_id : project.mesh_index->unmatched_faces) {
        if (!xray_faces.faces.count(face_id)) {
            gui_opengl_scene_mesh_face(
                project, callback, face_id, false, &scene);
        }
    }
    for (FaceId face_id : xray_faces.faces) {
        gui_opengl_scene_mesh_face(
            project, callback, face_id, true, &scene);
    }

    for (const auto &pair : project.select_node_objects) {
        bool xray = xray_node_object_names.count(pair.first);
        gui_opengl_scene_mesh_vertex(
            project, callback, pair.first, pair.second, xray, &scene);
    }
    for (const auto &pair : project.create_node_objects) {
        bool xray = xray_node_object_names.count(pair.first);
        gui_opengl_scene_mesh_vertex(
            project, callback, pair.first, pair.second, xray, &scene);
    }

    scene.animate_mode = animate_mode;
    scene.animate_hz = animate_hz;

    return std::make_shared<const GuiOpenglScene>(std::move(scene));
}

} /* namespace os2cx */
