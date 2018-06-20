#ifndef OS2CX_GUI_MAIN_WINDOW_HPP_
#define OX2CX_GUI_MAIN_WINDOW_HPP_

#include <QComboBox>
#include <QSplitter>
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
    void refresh_activity_combo_box();
    void refresh_activity(int new_index);

private:
    QSize sizeHint() const;
    void change_activity(QWidget *new_activity);
    void change_activity_to_progress_panel();
    GuiSceneAbstract::SceneParams scene_params();

    GuiProjectRunner *project_runner;
    GuiCameraSettings camera_settings;

    QSplitter *splitter;
    QWidget *left_panel;
    QComboBox *activity_combo_box;
    std::vector<std::function<void()> > activity_callbacks;
    int activity_first_result_index;
};

} /* namespace os2cx */

#endif
