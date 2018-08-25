#ifndef OS2CX_GUI_MODE_INSPECT_MESH_HPP_
#define OS2CX_GUI_MODE_INSPECT_MESH_HPP_

#include "gui_mode_inspect_abstract.hpp"
#include "gui_opengl_mesh.hpp"

namespace os2cx {

class GuiModeInspectMesh :
    public GuiModeInspectAbstract,
    private GuiOpenglMeshCallback
{
public:
    using GuiModeInspectAbstract::GuiModeInspectAbstract;

private:
    void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        Vector *displacement_out) const;

    std::shared_ptr<const GuiOpenglScene> make_scene();
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_INSPECT_MESH_HPP_ */
