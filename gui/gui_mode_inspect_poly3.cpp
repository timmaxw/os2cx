#include "gui_mode_inspect_poly3.hpp"

namespace os2cx {

void GuiModeInspectPoly3::calculate_attributes(
    const std::string &mesh_object_name,
    Plc3::SurfaceId surface_id,
    QColor *color_out
) const {
    (void)mesh_object_name;
    (void)surface_id;
    *color_out = QColor(0x80, 0x80, 0x80);
}

std::shared_ptr<const GuiOpenglScene> GuiModeInspectPoly3::make_scene() {
    return gui_opengl_scene_poly3(*project, this);
}

} /* namespace os2cx */
