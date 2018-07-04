#include "gui_mode_progress.hpp"

#include "gui_opengl_widget.hpp"

namespace os2cx {

GuiModeProgress::GuiModeProgress(QWidget *parent, const Project *project) :
    GuiModeAbstract(parent, project)
{
    create_widget_label(tr("Calculating..."));
    progress_bar = new QProgressBar(this);
    progress_bar->setMaximum(static_cast<int>(Project::Progress::AllDone));

    button_results = new QPushButton(tr("See results"), this);
    connect(
        button_results, &QPushButton::clicked,
        [this]() { emit see_results(); }
    );

    layout->addWidget(progress_bar, 0);
    layout->addWidget(button_results, 0, Qt::AlignRight);

    update_progress();
}

void GuiModeProgress::update_progress() {
    progress_bar->setValue(static_cast<int>(project->progress));
    button_results->setEnabled(
        project->progress == Project::Progress::ResultsDone);
}

std::shared_ptr<const GuiOpenglScene> GuiModeProgress::make_scene() {
    /* do nothing, show a blank page. TODO: do better */
    return std::make_shared<GuiOpenglScene>();
}

} /* namespace os2cx */