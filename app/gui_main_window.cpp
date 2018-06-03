#include "gui_main_window.hpp"

#include <iostream>

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

#include "gui_scene_mesh.hpp"
#include "gui_scene_poly3.hpp"

namespace os2cx {

GuiMainWindow::GuiMainWindow(const std::string &scad_path) :
    QMainWindow(nullptr),
    scene(nullptr)
{
    project_runner = new GuiProjectRunner(this, scad_path);

    QToolBar *tool_bar = addToolBar("toolbar");

    focus_combo_box = new GuiFocusComboBox(this, project_runner->get_project());
    tool_bar->addWidget(focus_combo_box);
    connect(
        project_runner, &GuiProjectRunner::project_updated,
        focus_combo_box, &GuiFocusComboBox::regenerate_options
    );
    connect(
        focus_combo_box, &GuiFocusComboBox::focus_changed,
        this, &GuiMainWindow::regenerate_scene
    );

    QMenu *menu_file = menuBar()->addMenu(tr("File"));
    menu_file->addAction(tr("Open"),
        this, &GuiMainWindow::menu_file_open);

    QMenu *menu_view = menuBar()->addMenu(tr("View"));
    action_show_elements = menu_view->addAction(tr("Show elements"));
    action_show_elements->setCheckable(true);
    connect(
        action_show_elements, &QAction::changed,
        this, &GuiMainWindow::regenerate_scene
    );

    resize(QSize(800, 600));

    regenerate_scene();
}

void GuiMainWindow::menu_file_open() {
    std::cout << "menu_file_open() not implemented yet" << std::endl;
}

void GuiMainWindow::regenerate_scene() {
    if (scene != nullptr) {
        delete scene;
        scene = nullptr;
    }

    GuiFocus focus = focus_combo_box->get_focus();
    bool show_elements = action_show_elements->isChecked();

    GuiSceneAbstract::SceneParams params;
    params.scene_parent = this;
    params.project = project_runner->get_project();
    params.camera_settings = &camera_settings;

    if (show_elements) {
        if (focus.type == GuiFocus::Type::All) {
            scene = new GuiSceneMesh(params);
        } else if (focus.type == GuiFocus::Type::Mesh ||
                focus.type == GuiFocus::Type::SelectVolume) {
            scene = new GuiSceneMeshVolume(params, focus.target);
        } else if (focus.type == GuiFocus::Type::Result) {
            scene = new GuiSceneMeshResultDisplacement(params, focus.target);
        } else {
            /* TODO: loads */
            scene = new GuiSceneMesh(params);
        }
    } else {
        if (focus.type == GuiFocus::Type::All) {
            scene = new GuiScenePoly3(params);
        } else if (focus.type == GuiFocus::Type::Mesh ||
                focus.type == GuiFocus::Type::SelectVolume) {
            scene = new GuiScenePoly3Volume(params, focus.target);
        } else {
            /* TODO: loads, results */
            scene = new GuiScenePoly3(params);
        }
    }
    setCentralWidget(scene);
}

} /* namespace os2cx */
