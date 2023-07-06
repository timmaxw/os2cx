#include "gui_main_window.hpp"

#include <iostream>

#include <QMenu>
#include <QMenuBar>

#include "gui_mode_inspect.hpp"
#include "gui_mode_progress.hpp"
#include "gui_mode_result.hpp"

namespace os2cx {

GuiMainWindow::GuiMainWindow(const std::string &scad_path_) :
    QMainWindow(nullptr),
    scad_path(scad_path_),
    current_mode(nullptr)
{
    setWindowTitle(scad_path_.c_str());

    QMenu *file_menu = menuBar()->addMenu(tr("File"));
    file_menu->addAction(tr("Open"),
        this, &GuiMainWindow::menu_file_open,
        QKeySequence::Open);
    file_menu->addAction(tr("Reload"),
        this, &GuiMainWindow::menu_file_reload,
        QKeySequence::Refresh);
    file_menu->addAction(tr("Interrupt"),
        this, &GuiMainWindow::menu_file_interrupt);

    splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    setCentralWidget(splitter);

    left_panel = new QWidget(nullptr);
    left_panel->setMinimumWidth(250);
    left_panel->setMaximumWidth(400);
    left_panel->setSizePolicy(
        QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    left_panel_layout = new QVBoxLayout(left_panel);
    left_panel->setLayout(left_panel_layout);
    splitter->addWidget(left_panel);
    splitter->setStretchFactor(0, 0);

    right_panel = new GuiOpenglWidget(this);
    splitter->addWidget(right_panel);
    splitter->setStretchFactor(1, 1);

    combo_box_modes = new GuiComboBoxModes(left_panel);
    left_panel_layout->addWidget(combo_box_modes);
    connect(
        combo_box_modes,
        &GuiComboBoxModes::current_mode_changed,
        this, &GuiMainWindow::set_current_mode);

    left_panel_layout->addStretch(1);

    initialize();
}

void GuiMainWindow::menu_file_open() {
    std::cout << "menu_file_open() not implemented yet" << std::endl;
}

void GuiMainWindow::menu_file_reload() {
    GuiProjectRunner::Status status = project_runner->status();
    if (status == GuiProjectRunner::Status::Running ||
            status == GuiProjectRunner::Status::Interrupting) {
        /* Wait for the project runner to stop before we reinitialize, but we
        can interrupt it so it stops sooner. */
        connect(project_runner.get(), &GuiProjectRunner::status_changed,
        [this](GuiProjectRunner::Status new_status) {
            if (new_status == GuiProjectRunner::Status::Interrupted) {
                initialize();
            }
        });
        project_runner->interrupt();
    } else {
        initialize();
    }
}

void GuiMainWindow::menu_file_interrupt() {
    project_runner->interrupt();
}

void GuiMainWindow::refresh_combo_box_modes() {
    std::shared_ptr<const Project> project = project_runner->get_project();

    GuiComboBoxModes::ModeVector modes;

    modes.push_back({
        tr("Progress"),
        [this]() {
            GuiModeProgress *mode = new GuiModeProgress(
                left_panel,
                project_runner);
            connect(
                mode, &GuiModeProgress::see_results,
                [this]() {
                    /* When there are multiple sets of results, assume the last
                    one is the most interesting one. E.g. in a steady state
                    dynamics analysis, there will be two sets of results: the
                    eigenmodes, then the actual steady state dynamics. */
                    combo_box_modes->set_current_mode(
                        final_result_mode_name);
                }
            );
            return mode;
        }
    });

    if (project->progress < Project::Progress::InventoryDone) {
        modes.push_back({tr("Inspect setup"), nullptr});
    } else {
        modes.push_back({
            tr("Inspect setup"),
            [this, project]() {
                GuiModeInspect *mode = new GuiModeInspect(
                    left_panel,
                    project);
                connect(
                    project_runner.get(), &GuiProjectRunner::project_updated,
                    mode, &GuiModeInspect::project_updated);
                return mode;
            }
        });
    }

    if (project->progress < Project::Progress::ResultsDone) {
        modes.push_back({tr("Results..."), nullptr});
    } else if (project->results->results.empty()) {
        modes.push_back({tr("No results emitted"), nullptr});
    } else {
        final_result_mode_name = QString();
        int i = 1;
        for (const Results::Result &result : project->results->results) {
            QString name;
            if (result.type == Results::Result::Type::Static) {
                name = tr("Result %1 (static)");
            } else if (result.type == Results::Result::Type::Eigenmode) {
                name = tr("Result %1 (eigenmodes)");
            } else if (result.type == Results::Result::Type::ModalDynamic) {
                name = tr("Result %1 (modal dynamic)");
            } else {
                assert(false);
            }
            name = name.arg(i);
            ++i;
            const Results::Result *result_ptr = &result;
            modes.push_back({
                name,
                [this, project, result_ptr]() {
                    return new GuiModeResult(left_panel, project, result_ptr);
                }
            });
            final_result_mode_name = name;
        }
    }

    combo_box_modes->set_modes(modes);
}

void GuiMainWindow::set_current_mode(GuiModeAbstract *new_mode) {
    GuiModeAbstract *old_mode = current_mode;
    current_mode = new_mode;

    if (old_mode != nullptr) {
        left_panel_layout->removeWidget(old_mode);
        old_mode->hide();
        disconnect(old_mode, nullptr, right_panel, nullptr);
    }

    if (new_mode != nullptr) {
        left_panel_layout->insertWidget(1, new_mode);
        new_mode->show();
        connect(new_mode, &GuiModeAbstract::refresh_scene,
            right_panel, &GuiOpenglWidget::refresh_scene);
    }

    right_panel->set_mode(new_mode);
}

void GuiMainWindow::initialize() {
    /* erase all existing modes in the combo box (because they can hold
    references to the old project) */
    combo_box_modes->clear();

    project_runner.reset(new GuiProjectRunner(this, scad_path));
    connect(project_runner.get(), &GuiProjectRunner::project_updated,
        this, &GuiMainWindow::refresh_combo_box_modes);

    refresh_combo_box_modes();
}

QSize GuiMainWindow::sizeHint() const {
    /* This default size is probably larger than the screen size, but Qt will
    automatically clamp it to something reasonable */
    return QSize(2000, 1500);
}

} /* namespace os2cx */
