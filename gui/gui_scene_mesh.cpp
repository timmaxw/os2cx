#include "gui_scene_mesh.hpp"

namespace os2cx {

void GuiSceneMesh::initialize_scene() {
    for (const FaceId &fi : project->mesh_index->unmatched_faces) {
        const Element3 &element = project->mesh->elements[fi.element_id];
        const ElementTypeInfo &type = ElementTypeInfo::get(element.type);
        NodeId node_ids[3];
        for (int i = 0; i < 3; ++i) {
            int vertex = type.shape->faces[fi.face].vertices[i];
            node_ids[i] = element.nodes[vertex];
        }

        Point ps[3];
        QColor cs[3];
        for (int i = 0; i < 3; ++i) {
            ps[i] = project->mesh->nodes[node_ids[i]].point;
            PureVector disp;
            calculate_attributes(
                fi.element_id, fi.face, node_ids[i], &cs[i], &disp);
            ps[i] += disp * Length(1.0);
        }

        add_triangle(ps, cs);

        for (int i = 0; i < 3; ++i) {
            int j = (i + 1) % 3;
            /* Every line is part of two triangles, but the two triangles
            traverse it in opposite directions. Only draw one of the two
            directions so that we don't draw each line twice. */
            if (node_ids[i].to_int() > node_ids[j].to_int()) {
                Point line_ps[2];
                line_ps[0] = ps[i];
                line_ps[1] = ps[j];
                add_line(line_ps);
            }
        }
    }
}

void GuiSceneMesh::calculate_attributes(
    ElementId element_id,
    int face_index,
    NodeId node_id,
    QColor *color_out,
    PureVector *displacement_out
) {
    (void)element_id;
    (void)face_index;
    (void)node_id;
    *color_out = QColor(0x80, 0x80, 0x80);
    *displacement_out = PureVector::zero();
}

} /* namespace os2cx */
