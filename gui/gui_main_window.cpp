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
    scene(nullptr),
    action_results_poly3(nullptr),
    action_results_mesh(nullptr)
{
    project_runner = new GuiProjectRunner(this, scad_path);
    connect(
        project_runner, &GuiProjectRunner::project_updated,
        this, &GuiMainWindow::regenerate_results_menu
    );

    QMenu *file_menu = menuBar()->addMenu(tr("File"));
    file_menu->addAction(tr("Open"),
        this, &GuiMainWindow::menu_file_open);

    menu_results = menuBar()->addMenu(tr("Results"));
    action_group_results = new QActionGroup(this);
    regenerate_results_menu();
    change_scene(new GuiScenePoly3(scene_params()));

    resize(QSize(800, 600));
}

void GuiMainWindow::menu_file_open() {
    std::cout << "menu_file_open() not implemented yet" << std::endl;
}

void GuiMainWindow::regenerate_results_menu() {
    const Project *project = project_runner->get_project();

    if (action_results_poly3 == nullptr) {
        action_results_poly3 = menu_results->addAction(
            tr("Pre-mesh geometry"),
            [this]() { change_scene(new GuiScenePoly3(scene_params())); });
        action_results_poly3->setActionGroup(action_group_results);
        action_results_poly3->setCheckable(true);
    }

    if (action_results_mesh == nullptr) {
        action_results_mesh = menu_results->addAction(
            tr("Post-mesh geometry"),
            [this]() { change_scene(new GuiSceneMesh(scene_params())); });
        action_results_mesh->setActionGroup(action_group_results);
        action_results_mesh->setCheckable(true);
    }

    if (project->results && action_results_results.empty()) {
        for (const auto &pair : project->results->node_vectors) {
            std::string name = pair.first;
            QAction *action_result_result = menu_results->addAction(
                tr("Result ") + QString(name.c_str()),
                [this, name]() {
                    change_scene(
                        new GuiSceneMeshResultDisplacement(
                            scene_params(), name));
                });
            action_result_result->setActionGroup(action_group_results);
            action_result_result->setCheckable(true);
            action_results_results.push_back(action_result_result);
        }
    }
}

GuiSceneAbstract::SceneParams GuiMainWindow::scene_params() {
    GuiSceneAbstract::SceneParams params;
    params.scene_parent = this;
    params.project = project_runner->get_project();
    params.camera_settings = &camera_settings;
    return params;
}

void GuiMainWindow::change_scene(GuiSceneAbstract *new_scene) {
    if (scene != nullptr) {
        delete scene;
        scene = nullptr;
    }
    scene = new_scene;
    setCentralWidget(scene);
}

} /* namespace os2cx */
