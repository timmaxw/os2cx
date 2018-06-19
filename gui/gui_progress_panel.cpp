#include "gui_progress_panel.hpp"

#include <QVBoxLayout>

namespace os2cx {

GuiProgressPanel::GuiProgressPanel(QWidget *parent, const Project *project) :
    QWidget(parent), project(project)
{
    progress_bar = new QProgressBar(this);
    progress_bar->setMaximum(static_cast<int>(Project::Progress::AllDone));

    button_results = new QPushButton(tr("See results"), this);
    connect(
        button_results, &QPushButton::clicked,
        [this]() { emit see_results(); }
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch(1);
    layout->addWidget(progress_bar, 0);
    layout->addStretch(1);
    layout->addWidget(button_results, 0, Qt::AlignRight);
    setLayout(layout);

    update();
}

void GuiProgressPanel::update() {
    progress_bar->setValue(static_cast<int>(project->progress));
    button_results->setEnabled(
        project->progress == Project::Progress::ResultsDone);
}

} /* namespace os2cx */
