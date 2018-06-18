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
    GuiSceneAbstract::SceneParams scene_params();
    void change_scene(GuiSceneAbstract *new_scene);

    GuiProjectRunner *project_runner;
    GuiCameraSettings camera_settings;
    GuiSceneAbstract *scene;

    QMenu *menu_results;
    QActionGroup *action_group_results;
    QAction *action_results_poly3;
    QAction *action_results_mesh;
    std::vector<QAction *> action_results_results;
};

} /* namespace os2cx */

#endif
