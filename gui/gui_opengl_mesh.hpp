#ifndef GUI_OPENGL_MESH_HPP
#define GUI_OPENGL_MESH_HPP

#include "gui_opengl_widget.hpp"

namespace os2cx {

class GuiOpenglMeshCallback
{
public:
    virtual void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        Vector *displacement_out) const = 0;
};

std::shared_ptr<const GuiOpenglScene> gui_opengl_scene_mesh(
    const Project &project,
    const GuiOpenglMeshCallback *callback);

} /* namespace os2cx */

#endif // GUI_OPENGL_MESH_HPP
