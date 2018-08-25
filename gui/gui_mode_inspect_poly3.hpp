#ifndef OS2CX_GUI_MODE_INSPECT_POLY3_HPP_
#define OS2CX_GUI_MODE_INSPECT_POLY3_HPP_

#include "gui_mode_inspect_abstract.hpp"
#include "gui_opengl_poly3.hpp"

namespace os2cx {

class GuiModeInspectPoly3 :
    public GuiModeInspectAbstract,
    public GuiOpenglPoly3Callback
{
public:
    using GuiModeInspectAbstract::GuiModeInspectAbstract;

private:
    void calculate_attributes(
        const std::string &mesh_object_name,
        Plc3::SurfaceId surface_id,
        QColor *color_out) const;

    std::shared_ptr<const GuiOpenglScene> make_scene();
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_INSPECT_POLY3_HPP_ */
