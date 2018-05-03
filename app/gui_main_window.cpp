#include "gui_main_window.hpp"

#include <iostream>

#include <QMenu>
#include <QMenuBar>

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

    QMenu *menu_file = menuBar()->addMenu(tr("File"));
    menu_file->addAction(tr("Open"), this, &GuiMainWindow::menu_file_open);
}

void GuiMainWindow::menu_file_open() {
    std::cout << "menu_file_open() not implemented yet" << std::endl;
}

} /* namespace os2cx */
