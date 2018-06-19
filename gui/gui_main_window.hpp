#ifndef OS2CX_GUI_MAIN_WINDOW_HPP_
#define OX2CX_GUI_MAIN_WINDOW_HPP_

#include <QActionGroup>
#include <QMainWindow>

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
    void regenerate_results_menu();

private:
    void change_central_widget(QWidget *new_central_widget);
    void change_central_widget_to_progress_panel();
    GuiSceneAbstract::SceneParams scene_params();

    GuiProjectRunner *project_runner;
    GuiCameraSettings camera_settings;

    QMenu *menu_results;
    QActionGroup *action_group_results;
    QAction *action_first_result;
};

} /* namespace os2cx */

#endif
