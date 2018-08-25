#ifndef OS2CX_GUI_MODE_INSPECT_ABSTRACT_HPP_
#define OS2CX_GUI_MODE_INSPECT_ABSTRACT_HPP_

#include "gui_mode_abstract.hpp"

namespace os2cx {

class GuiModeInspectAbstract : public GuiModeAbstract
{
public:
    GuiModeInspectAbstract(
        QWidget *parent,
        std::shared_ptr<const Project> project);
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_INSPECT_ABSTRACT_HPP_ */
