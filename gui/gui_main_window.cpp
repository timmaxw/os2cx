#include "gui_main_window.hpp"

#include <iostream>

#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "gui_progress_panel.hpp"
#include "gui_scene_mesh.hpp"
#include "gui_scene_poly3.hpp"

namespace os2cx {

GuiMainWindow::GuiMainWindow(const std::string &scad_path) :
    QMainWindow(nullptr),
    activity_first_result_index(0)
{
    project_runner = new GuiProjectRunner(this, scad_path);
    connect(project_runner, &GuiProjectRunner::project_updated,
        this, &GuiMainWindow::refresh_activity_combo_box);

    QMenu *file_menu = menuBar()->addMenu(tr("File"));
    file_menu->addAction(tr("Open"),
        this, &GuiMainWindow::menu_file_open);

    splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    setCentralWidget(splitter);

    left_panel = new QWidget(nullptr);
    left_panel->setMinimumWidth(200);
    left_panel->setMaximumWidth(400);
    QVBoxLayout *left_panel_layout = new QVBoxLayout(left_panel);
    left_panel->setLayout(left_panel_layout);
    splitter->addWidget(left_panel);
    splitter->setStretchFactor(0, 0);

    activity_combo_box = new QComboBox(left_panel);
    left_panel_layout->addWidget(activity_combo_box);
    refresh_activity_combo_box();
    connect(activity_combo_box, QOverload<int>::of(&QComboBox::activated),
        this, &GuiMainWindow::refresh_activity);

    left_panel_layout->addStretch(1);

    refresh_activity(activity_combo_box->currentIndex());
}

void GuiMainWindow::menu_file_open() {
    std::cout << "menu_file_open() not implemented yet" << std::endl;
}

void GuiMainWindow::refresh_activity_combo_box() {
    QString prev_selected = activity_combo_box->currentText();

    QStandardItemModel *activity_item_model =
        static_cast<QStandardItemModel *>(activity_combo_box->model());
    activity_item_model->clear();
    activity_callbacks.clear();
    auto add_item = [&](
        const QString &text,
        const std::function<void()> &cb,
        bool enabled
    ) {
        int index = activity_callbacks.size();
        QStandardItem *item = new QStandardItem(text);
        item->setEnabled(enabled);
        activity_item_model->appendRow(item);
        activity_callbacks.push_back(cb);
        if (prev_selected == text) {
            activity_combo_box->setCurrentIndex(index);
        }
        return index;
    };

    const Project *project = project_runner->get_project();

    add_item(
        tr("Progress"),
        [this]() { change_activity_to_progress_panel(); },
        true);

    add_item(
        tr("Pre-mesh geometry"),
        [this]() { change_activity(new GuiScenePoly3(scene_params())); },
        project->progress >= Project::Progress::PolyAttrsDone);

    activity_first_result_index = add_item(
        tr("Post-mesh geometry"),
        [this]() { change_activity(new GuiSceneMesh(scene_params())); },
        project->progress >= Project::Progress::MeshAttrsDone);

    if (project->progress < Project::Progress::ResultsDone) {
        add_item(tr("Results..."), [](){}, false);
    } else {
        bool is_first = true;
        for (const auto &pair : project->results->node_vectors) {
            std::string name = pair.first;
            int index = add_item(
                tr("Result ") + QString(name.c_str()),
                [this, name]() {
                    change_activity(
                        new GuiSceneMeshResultDisplacement(
                            scene_params(), name));
                },
                true);
            if (is_first) {
                activity_first_result_index = index;
                is_first = false;
            }
        }
    }
}

void GuiMainWindow::refresh_activity(int new_index) {
    if (new_index != -1) {
        activity_callbacks[new_index]();
    }
}

QSize GuiMainWindow::sizeHint() const {
    return QSize(10000, 10000);
}

void GuiMainWindow::change_activity(QWidget *new_activity) {
    if (splitter->count() == 1) {
        splitter->addWidget(new_activity);
    } else {
        QWidget *old_activity = splitter->replaceWidget(1, new_activity);
        delete old_activity;
    }
    splitter->setStretchFactor(1, 1);
}

void GuiMainWindow::change_activity_to_progress_panel() {
    GuiProgressPanel *progress_panel = new GuiProgressPanel(
        this, project_runner->get_project());
    connect(
        project_runner, &GuiProjectRunner::project_updated,
        progress_panel, &GuiProgressPanel::update
    );
    connect(
        progress_panel, &GuiProgressPanel::see_results,
        [this]() {
            activity_combo_box->setCurrentIndex(activity_first_result_index);
            refresh_activity(activity_first_result_index);
        }
    );
    change_activity(progress_panel);
}

GuiSceneAbstract::SceneParams GuiMainWindow::scene_params() {
    GuiSceneAbstract::SceneParams params;
    params.scene_parent = this;
    params.project = project_runner->get_project();
    params.camera_settings = &camera_settings;
    return params;
}

} /* namespace os2cx */
