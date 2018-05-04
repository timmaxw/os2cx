#ifndef GUI_MAIN_WINDOW_HPP_
#define GUI_MAIN_WINDOW_HPP_

#include <QMainWindow>

#include "gui_focus_combo_box.hpp"
#include "gui_opengl_widget.hpp"
#include "gui_project_runner.hpp"
#include "project.hpp"

namespace os2cx {

class GuiMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GuiMainWindow(const std::string &scad_path);

public slots:
    void menu_file_open();

private:
    std::unique_ptr<Project> project;
    GuiProjectRunner *project_runner;
    GuiFocusComboBox *focus_combo_box;
    GuiOpenglWidget *opengl_widget;
};

} /* namespace os2cx */

#endif /* GUI_MAIN_WINDOW_HPP_ */
