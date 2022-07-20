#ifndef OS2CX_GUI_MODE_PROGRESS_HPP_
#define OS2CX_GUI_MODE_PROGRESS_HPP_

#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

#include "gui_mode_abstract.hpp"
#include "gui_opengl_mesh.hpp"
#include "gui_opengl_poly3.hpp"
#include "gui_project_runner.hpp"

namespace os2cx {

class GuiModeProgress :
    public GuiModeAbstract,
    public GuiOpenglMeshCallback,
    public GuiOpenglPoly3Callback
{
    Q_OBJECT
public:
    GuiModeProgress(
        QWidget *parent,
        std::shared_ptr<const GuiProjectRunner> project_runner);

signals:
    void see_results();

private slots:
    void project_logged();
    void project_updated();

private:
    void calculate_surface_attributes(
        const std::string &mesh_object_name,
        Plc3::SurfaceId surface_id,
        QColor *color_out,
        bool *xray_out) const;

    void calculate_vertex_attributes(
        const std::string &node_object_name,
        QColor *vertex_color_out,
        bool *xray_out) const;

    void calculate_face_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        ComplexVector *displacement_out,
        QColor *color_out,
        bool *xray_out) const;

    void calculate_vertex_attributes(
        const std::string &node_object_name,
        ComplexVector *displacement_out,
        QColor *vertex_color_out,
        bool *xray_out) const;

    std::shared_ptr<const GuiOpenglScene> make_scene();

    std::shared_ptr<const GuiProjectRunner> project_runner;

    int last_log_line;
    QTextEdit *log_text;

    QProgressBar *progress_bar;
    QPushButton *button_results;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_PROGRESS_HPP_ */
