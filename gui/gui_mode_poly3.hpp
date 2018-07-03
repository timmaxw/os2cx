#ifndef OS2CX_GUI_MODE_POLY3_HPP_
#define OS2CX_GUI_MODE_POLY3_HPP_

#include "gui_mode_abstract.hpp"

namespace os2cx {

class GuiModePoly3 : public GuiModeAbstract
{
public:
    using GuiModeAbstract::GuiModeAbstract;

protected:
    std::shared_ptr<const GuiOpenglTriangles> make_triangles();
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_POLY3_HPP_ */
