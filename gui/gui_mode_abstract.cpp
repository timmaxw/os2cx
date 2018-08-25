#include "gui_mode_abstract.hpp"

namespace os2cx {

GuiModeAbstract::GuiModeAbstract(
    QWidget *parent,
    std::shared_ptr<const Project> project_
) :
    QWidget(parent), project(project_)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    /* Disable the inner margins because we'll be embedded in the 'left_panel'
    widget that has its own margins */
    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

QLabel *GuiModeAbstract::create_widget_label(const QString &text) {
    layout->addSpacing(10);
    QLabel *label = new QLabel(text, this);
    layout->addWidget(label);
    return label;
}

} /* namespace os2cx */
