#include "gui_mode_inspect_mesh.hpp"

#include "gui_opengl_widget.hpp"

namespace os2cx {

void GuiModeInspectMesh::calculate_attributes(
    ElementId element_id,
    int face_index,
    NodeId node_id,
    QColor *color_out,
    Vector *displacement_out
) const {
    (void)element_id;
    (void)face_index;
    (void)node_id;
    *color_out = QColor(0x80, 0x80, 0x80);
    *displacement_out = Vector::zero();
}

std::shared_ptr<const GuiOpenglScene> GuiModeInspectMesh::make_scene() {
    return gui_opengl_scene_mesh(*project, this);
}

} /* namespace os2cx */
