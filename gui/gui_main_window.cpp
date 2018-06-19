#include "gui_main_window.hpp"

#include <iostream>

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

#include "gui_progress_panel.hpp"
#include "gui_scene_mesh.hpp"
#include "gui_scene_poly3.hpp"

namespace os2cx {

GuiMainWindow::GuiMainWindow(const std::string &scad_path) :
    QMainWindow(nullptr),
    action_first_result(nullptr)
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
    change_central_widget_to_progress_panel();

    resize(QSize(800, 600));
}

void GuiMainWindow::menu_file_open() {
    std::cout << "menu_file_open() not implemented yet" << std::endl;
}

void GuiMainWindow::regenerate_results_menu() {
    QString prev_selected = action_group_results->checkedAction() ?
        action_group_results->checkedAction()->text() : tr("Progress");
    menu_results->clear();
    auto add_item = [&](
        const QString &text,
        const std::function<void()> &cb,
        bool enabled
    ) {
        QAction *action = menu_results->addAction(text, cb);
        action->setActionGroup(action_group_results);
        action->setDisabled(!enabled);
        action->setCheckable(true);
        if (text == prev_selected) {
            action->setChecked(true);
        }
        return action;
    };

    const Project *project = project_runner->get_project();

    add_item(
        tr("Progress"),
        [this]() { change_central_widget_to_progress_panel(); },
        true);

    add_item(
        tr("Pre-mesh geometry"),
        [this]() { change_central_widget(new GuiScenePoly3(scene_params())); },
        project->progress >= Project::Progress::PolyAttrsDone);

    action_first_result = add_item(
        tr("Post-mesh geometry"),
        [this]() { change_central_widget(new GuiSceneMesh(scene_params())); },
        project->progress >= Project::Progress::MeshAttrsDone);

    if (project->progress < Project::Progress::ResultsDone) {
        add_item(tr("Results..."), [](){}, false);
    } else {
        bool is_first = true;
        for (const auto &pair : project->results->node_vectors) {
            std::string name = pair.first;
            QAction *action = add_item(
                tr("Result ") + QString(name.c_str()),
                [this, name]() {
                    change_central_widget(
                        new GuiSceneMeshResultDisplacement(
                            scene_params(), name));
                },
                true);
            if (is_first) {
                action_first_result = action;
                is_first = false;
            }
        }
    }
}

void GuiMainWindow::change_central_widget(QWidget *new_central_widget) {
    if (centralWidget() != nullptr) {
        delete centralWidget();
    }
    setCentralWidget(new_central_widget);
}

void GuiMainWindow::change_central_widget_to_progress_panel() {
    GuiProgressPanel *progress_panel = new GuiProgressPanel(
        this, project_runner->get_project());
    connect(
        project_runner, &GuiProjectRunner::project_updated,
        progress_panel, &GuiProgressPanel::update
    );
    connect(
        progress_panel, &GuiProgressPanel::see_results,
        [this]() {
            if (action_first_result != nullptr) {
                action_first_result->activate(QAction::Trigger);
            }
        }
    );
    change_central_widget(progress_panel);
}

GuiSceneAbstract::SceneParams GuiMainWindow::scene_params() {
    GuiSceneAbstract::SceneParams params;
    params.scene_parent = this;
    params.project = project_runner->get_project();
    params.camera_settings = &camera_settings;
    return params;
}

} /* namespace os2cx */
