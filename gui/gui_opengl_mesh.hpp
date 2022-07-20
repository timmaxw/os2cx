#ifndef GUI_OPENGL_MESH_HPP
#define GUI_OPENGL_MESH_HPP

#include "gui_opengl_widget.hpp"

namespace os2cx {

class GuiOpenglMeshCallback
{
public:
    virtual void calculate_xrays(
        FaceSet *xray_faces_out,
        std::set<std::string> *xray_node_object_names_out) const = 0;

    virtual void calculate_face_attributes(
        FaceId face_id,
        NodeId node_id,
        ComplexVector *displacement_out,
        QColor *color_out) const = 0;

    virtual void calculate_vertex_attributes(
        const std::string &node_object_name,
        ComplexVector *displacement_out,
        QColor *color_out) const = 0;
};

std::shared_ptr<const GuiOpenglScene> gui_opengl_scene_mesh(
    const Project &project,
    const GuiOpenglMeshCallback *callback,
    const GuiOpenglScene::AnimateMode animate_mode
        = GuiOpenglScene::AnimateMode::None,
    double animate_hz = 0.0
);

} /* namespace os2cx */

#endif // GUI_OPENGL_MESH_HPP
