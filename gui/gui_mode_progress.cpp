#include "gui_mode_progress.hpp"

#include "gui_opengl_widget.hpp"

namespace os2cx {

GuiModeProgress::GuiModeProgress(
    QWidget *parent,
    std::shared_ptr<const GuiProjectRunner> project_runner
) :
    GuiModeAbstract(parent, project_runner->get_project()),
    project_runner(project_runner)
{
    create_widget_label(tr("Calculation log"));
    last_log_line = 0;
    log_text = new QTextEdit(this);
    layout->addWidget(log_text, 1);

    create_widget_label(tr("Calculation progress"));
    progress_bar = new QProgressBar(this);
    progress_bar->setMaximum(static_cast<int>(Project::Progress::AllDone));
    layout->addWidget(progress_bar, 0);

    button_results = new QPushButton(tr("See results"), this);
    connect(
        button_results, &QPushButton::clicked,
        [this]() { emit see_results(); }
    );
    layout->addWidget(button_results, 0, Qt::AlignRight);

    connect(
        project_runner.get(), &GuiProjectRunner::project_logged,
        this, &GuiModeProgress::project_logged);
    connect(
        project_runner.get(), &GuiProjectRunner::project_updated,
        this, &GuiModeProgress::project_updated);
    connect(
        project_runner.get(), &GuiProjectRunner::status_changed,
        this, &GuiModeProgress::project_updated);
    project_updated();
}

void GuiModeProgress::project_logged() {
    for (; last_log_line < static_cast<int>(project_runner->logs.size());
            ++last_log_line) {
        log_text->append(project_runner->logs.at(last_log_line));
    }
}

void GuiModeProgress::project_updated() {
    GuiProjectRunner::Status status = project_runner->status();

    progress_bar->setValue(static_cast<int>(project->progress));
    progress_bar->setEnabled(
        status == GuiProjectRunner::Status::Running ||
            status == GuiProjectRunner::Status::Done);

    button_results->setEnabled(status == GuiProjectRunner::Status::Done);

    emit refresh_scene();
}

void GuiModeProgress::calculate_xrays(
    std::set<std::pair<std::string, Plc3::SurfaceId> > *xray_surfaces_out,
    std::set<std::string> *xray_node_object_names_out
) const {
    (void)xray_surfaces_out;
    (void)xray_node_object_names_out;
}

void GuiModeProgress::calculate_surface_attributes(
    const std::string &mesh_object_name,
    Plc3::SurfaceId surface_id,
    QColor *color_out
) const {
    (void)mesh_object_name;
    (void)surface_id;
    *color_out = QColor(0x80, 0x80, 0x80);
}

void GuiModeProgress::calculate_vertex_attributes(
    const std::string &node_object_name,
    QColor *color_out
) const {
    (void)node_object_name;
    *color_out = QColor(0x70, 0x70, 0x70);
}

void GuiModeProgress::calculate_xrays(
    FaceSet *xray_faces_out,
    std::set<std::string> *xray_node_object_names_out
) const {
    (void)xray_faces_out;
    (void)xray_node_object_names_out;
}

void GuiModeProgress::calculate_face_attributes(
    FaceId face_id,
    NodeId node_id,
    ComplexVector *displacement_out,
    QColor *color_out
) const {
    (void)face_id,
    (void)node_id;
    *displacement_out = ComplexVector::zero();
    *color_out = QColor(0x80, 0x80, 0x80);
}

void GuiModeProgress::calculate_vertex_attributes(
    const std::string &node_object_name,
    ComplexVector *displacement_out,
    QColor *color_out
) const {
    (void)node_object_name;
    *displacement_out = ComplexVector::zero();
    *color_out = QColor(0x70, 0x70, 0x70);
}

std::shared_ptr<const GuiOpenglScene> GuiModeProgress::make_scene() {
    if (project->progress < Project::Progress::PolyAttrsDone) {
        return std::make_shared<const GuiOpenglScene>();
    } else if (project->progress < Project::Progress::MeshAttrsDone) {
        return gui_opengl_scene_poly3(*project, this);
    } else {
        return gui_opengl_scene_mesh(*project, this);
    }
}

} /* namespace os2cx */
