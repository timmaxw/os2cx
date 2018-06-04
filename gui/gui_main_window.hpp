#ifndef OS2CX_GUI_MAIN_WINDOW_HPP_
#define OX2CX_GUI_MAIN_WINDOW_HPP_

#include <QMainWindow>

#include "gui_focus_combo_box.hpp"
#include "gui_scene_abstract.hpp"
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
    void regenerate_scene();

private:
    GuiProjectRunner *project_runner;
    GuiFocusComboBox *focus_combo_box;
    GuiCameraSettings camera_settings;
    GuiSceneAbstract *scene;
    QAction *action_show_elements;
};

} /* namespace os2cx */

#endif
