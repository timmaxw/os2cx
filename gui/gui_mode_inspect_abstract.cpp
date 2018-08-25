#include "gui_mode_inspect_abstract.hpp"

#include <QProgressBar>

namespace os2cx {

GuiModeInspectAbstract::GuiModeInspectAbstract(
    QWidget *parent,
    std::shared_ptr<const Project> project
) :
    GuiModeAbstract(parent, project)
{
    create_widget_label(tr("Inspect..."));
}

} /* namespace os2cx */
