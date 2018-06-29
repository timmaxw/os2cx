#include "gui_main_window.hpp"

#include <iostream>

#include <QMenu>
#include <QMenuBar>

#include "gui_scene_mesh.hpp"
#include "gui_scene_poly3.hpp"
#include "gui_scene_progress.hpp"
#include "gui_scene_result.hpp"

namespace os2cx {

GuiMainWindow::GuiMainWindow(const std::string &scad_path) :
    QMainWindow(nullptr),
    current_scene(nullptr)
{
    project_runner = new GuiProjectRunner(this, scad_path);
    connect(project_runner, &GuiProjectRunner::project_updated,
        this, &GuiMainWindow::refresh_combo_box_scenes);

    QMenu *file_menu = menuBar()->addMenu(tr("File"));
    file_menu->addAction(tr("Open"),
        this, &GuiMainWindow::menu_file_open);

    splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    setCentralWidget(splitter);

    left_panel = new QWidget(nullptr);
    left_panel->setMinimumWidth(200);
    left_panel->setMaximumWidth(400);
    left_panel_layout = new QVBoxLayout(left_panel);
    left_panel->setLayout(left_panel_layout);
    splitter->addWidget(left_panel);
    splitter->setStretchFactor(0, 0);

    right_panel = new GuiOpenglWidget(this, project_runner->get_project());
    splitter->addWidget(right_panel);

    combo_box_scenes = new GuiComboBoxScenes(left_panel);
    left_panel_layout->addWidget(combo_box_scenes);
    connect(
        combo_box_scenes,
        &GuiComboBoxScenes::current_scene_changed,
        this, &GuiMainWindow::set_current_scene);
    refresh_combo_box_scenes();

    left_panel_layout->addStretch(1);

}

void GuiMainWindow::menu_file_open() {
    std::cout << "menu_file_open() not implemented yet" << std::endl;
}

void GuiMainWindow::refresh_combo_box_scenes() {
    const Project *project = project_runner->get_project();

    GuiComboBoxScenes::SceneVector scenes;

    scenes.push_back({
        tr("Progress"),
        [this]() {
            GuiSceneProgress *scene = new GuiSceneProgress(
                left_panel,
                project_runner->get_project());
            connect(
                project_runner, &GuiProjectRunner::project_updated,
                scene, &GuiSceneProgress::update_progress
            );
            connect(
                scene, &GuiSceneProgress::see_results,
                [this]() {
                    combo_box_scenes->set_current_scene(
                        first_result_scene_name);
                }
            );
            return scene;
        }
    });

    if (project->progress >= Project::Progress::PolyAttrsDone) {
        scenes.push_back({
            tr("Pre-mesh geometry"),
            [this]() {
                return new GuiScenePoly3(
                    left_panel,
                    project_runner->get_project());
            }
        });
    } else {
        scenes.push_back({tr("Pre-mesh geometry"), nullptr});
    }

    if (project->progress >= Project::Progress::MeshAttrsDone) {
        scenes.push_back({
            tr("Mesh geometry"),
            [this]() {
                return new GuiSceneMesh(
                    left_panel,
                    project_runner->get_project());
            }
        });
    } else {
        scenes.push_back({tr("Mesh geometry"), nullptr});
    }

    if (project->progress < Project::Progress::ResultsDone) {
        scenes.push_back({tr("Results..."), nullptr});
    } else {
        first_result_scene_name = QString();
        for (const auto &pair : project->results->static_steps) {
            std::string result_name = pair.first;
            QString name = tr("Result ") + QString(result_name.c_str());
            scenes.push_back({
                name,
                [this, result_name]() {
                    return new GuiSceneResultStatic(
                        left_panel,
                        project_runner->get_project(),
                        result_name);
                }
            });
            if (first_result_scene_name.isNull()) {
                first_result_scene_name = name;
            }
        }
    }

    combo_box_scenes->set_scenes(scenes);
}

void GuiMainWindow::set_current_scene(GuiSceneAbstract *new_scene) {
    GuiSceneAbstract *old_scene = current_scene;
    current_scene = new_scene;

    if (old_scene != nullptr) {
        left_panel_layout->removeWidget(old_scene);
        old_scene->hide();
        disconnect(old_scene, nullptr, right_panel, nullptr);
    }

    if (new_scene != nullptr) {
        left_panel_layout->insertWidget(1, new_scene);
        new_scene->show();
        connect(new_scene, &GuiSceneAbstract::rerender,
            right_panel, QOverload<>::of(&GuiOpenglWidget::update));
    }

    right_panel->set_scene(new_scene);
}

QSize GuiMainWindow::sizeHint() const {
    /* This default size is probably larger than the screen size, but Qt will
    automatically clamp it to something reasonable */
    return QSize(2000, 1500);
}

} /* namespace os2cx */
