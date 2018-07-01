#ifndef OS2CX_GUI_POLY3_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_POLY3_OPENGL_WIDGET_HPP_

#include "gui_scene_abstract.hpp"

namespace os2cx {

class GuiScenePoly3 : public GuiSceneAbstract
{
public:
    using GuiSceneAbstract::GuiSceneAbstract;

protected:
    std::shared_ptr<const GuiOpenglTriangles> make_triangles();
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_POLY3_OPENGL_WIDGET_HPP_ */
