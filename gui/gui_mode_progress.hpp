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
    void calculate_attributes(
        const std::string &mesh_object_name,
        Plc3::SurfaceId surface_id,
        QColor *color_out) const;

    void calculate_attributes(
        ElementId element_id,
        int face_index,
        NodeId node_id,
        QColor *color_out,
        ComplexVector *displacement_out) const;

    std::shared_ptr<const GuiOpenglScene> make_scene();

    std::shared_ptr<const GuiProjectRunner> project_runner;

    int last_log_line;
    QTextEdit *log_text;

    QProgressBar *progress_bar;
    QPushButton *button_results;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_PROGRESS_HPP_ */
