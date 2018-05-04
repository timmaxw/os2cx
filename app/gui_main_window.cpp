#include "gui_main_window.hpp"

#include <iostream>

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

namespace os2cx {

GuiMainWindow::GuiMainWindow(const std::string &scad_path) :
    QMainWindow(nullptr)
{
    project_runner = new GuiProjectRunner(this, scad_path);

    opengl_widget = new GuiOpenglWidget(this, project_runner->get_project());
    setCentralWidget(opengl_widget);
    connect(
        project_runner, &GuiProjectRunner::project_updated,
        opengl_widget, &GuiOpenglWidget::regenerate_scene
    );

    QToolBar *tool_bar = addToolBar("toolbar");

    focus_combo_box = new GuiFocusComboBox(this, project_runner->get_project());
    tool_bar->addWidget(focus_combo_box);
    connect(
        project_runner, &GuiProjectRunner::project_updated,
        focus_combo_box, &GuiFocusComboBox::regenerate_options
    );
    connect(focus_combo_box, &GuiFocusComboBox::focus_changed,
        [this]() {
            opengl_widget->scene_settings.focus = focus_combo_box->get_focus();
            opengl_widget->regenerate_scene();
        }
    );

    QMenu *menu_file = menuBar()->addMenu(tr("File"));
    menu_file->addAction(tr("Open"), this, &GuiMainWindow::menu_file_open);
}

void GuiMainWindow::menu_file_open() {
    std::cout << "menu_file_open() not implemented yet" << std::endl;
}

} /* namespace os2cx */
