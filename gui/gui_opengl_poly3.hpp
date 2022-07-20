#ifndef GUI_OPENGL_POLY3_HPP
#define GUI_OPENGL_POLY3_HPP

#include "gui_opengl_widget.hpp"

namespace os2cx {

class GuiOpenglPoly3Callback
{
public:
    virtual void calculate_xrays(
        std::set<std::pair<std::string, Plc3::SurfaceId> > *xray_surfaces_out,
        std::set<std::string> *xray_node_object_names_out) const = 0;

    virtual void calculate_surface_attributes(
        const std::string &mesh_object_name,
        Plc3::SurfaceId surface_id,
        QColor *color_out) const = 0;

    virtual void calculate_vertex_attributes(
        const std::string &node_object_name,
        QColor *color_out) const = 0;
};

std::shared_ptr<const GuiOpenglScene> gui_opengl_scene_poly3(
    const Project &project,
    const GuiOpenglPoly3Callback *callback);

} /* namespace os2cx */

#endif // GUI_OPENGL_MESH_HPP
