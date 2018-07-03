#ifndef OS2CX_GUI_MODE_MESH_HPP_
#define OS2CX_GUI_MODE_MESH_HPP_

#include "gui_mode_abstract.hpp"

namespace os2cx {

class GuiModeMesh : public GuiModeAbstract
{
public:
    using GuiModeAbstract::GuiModeAbstract;

protected:
    std::shared_ptr<const GuiOpenglTriangles> make_triangles();

    virtual void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        Vector *displacement_out);
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_MESH_HPP_ */
