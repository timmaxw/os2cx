#include "gui_mode_progress.hpp"

#include "gui_opengl_widget.hpp"

namespace os2cx {

GuiModeProgress::GuiModeProgress(
    QWidget *parent,
    const GuiProjectRunner *project_runner
) :
    GuiModeAbstract(parent, project_runner->get_project()),
    project_runner(project_runner)
{
    label = create_widget_label(tr("Calculating..."));

    progress_bar = new QProgressBar(this);
    progress_bar->setMaximum(static_cast<int>(Project::Progress::AllDone));

    button_results = new QPushButton(tr("See results"), this);
    connect(
        button_results, &QPushButton::clicked,
        [this]() { emit see_results(); }
    );

    layout->addWidget(progress_bar, 0);
    layout->addWidget(button_results, 0, Qt::AlignRight);

    connect(
        project_runner, &GuiProjectRunner::project_updated,
        this, &GuiModeProgress::update_progress);
    connect(
        project_runner, &GuiProjectRunner::status_changed,
        this, &GuiModeProgress::update_progress);
    update_progress();
}

void GuiModeProgress::update_progress() {
    GuiProjectRunner::Status status = project_runner->status();
    switch (status) {
    case GuiProjectRunner::Status::Running:
        switch (project->progress) {
        case Project::Progress::NothingDone:
        case Project::Progress::InventoryDone:
            label->setText(tr("Running OpenSCAD..."));
            break;
        case Project::Progress::PolysDone:
            label->setText(tr("Preparing pre-mesh geometry..."));
            break;
        case Project::Progress::PolyAttrsDone:
            label->setText(tr("Meshing..."));
            break;
        case Project::Progress::MeshDone:
            label->setText(tr("Preparing mesh geometry..."));
            break;
        case Project::Progress::MeshAttrsDone:
            label->setText(tr("Running CalculiX..."));
            break;
        default: assert(false);
        }
        break;

    case GuiProjectRunner::Status::Done:
        label->setText(tr("Calculation finished."));
        break;

    case GuiProjectRunner::Status::Interrupting:
        label->setText(tr("Interrupting calculation..."));
        break;

    case GuiProjectRunner::Status::Interrupted:
        label->setText(tr("Calculation interrupted."));
        break;

    default: assert(false);
    }

    progress_bar->setValue(static_cast<int>(project->progress));
    progress_bar->setEnabled(
        status == GuiProjectRunner::Status::Running ||
            status == GuiProjectRunner::Status::Done);

    button_results->setEnabled(status == GuiProjectRunner::Status::Done);
}

std::shared_ptr<const GuiOpenglScene> GuiModeProgress::make_scene() {
    /* do nothing, show a blank page. TODO: do better */
    return std::make_shared<GuiOpenglScene>();
}

} /* namespace os2cx */
