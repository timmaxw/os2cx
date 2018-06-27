#ifndef OS2CX_GUI_ABSTRACT_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_ABSTRACT_OPENGL_WIDGET_HPP_

#include <QOpenGLFunctions_1_1>
#include <QVBoxLayout>
#include <QWidget>

#include "project.hpp"

namespace os2cx {

class GuiSceneAbstract : public QWidget
{
    Q_OBJECT
public:
    GuiSceneAbstract(QWidget *parent, const Project *project);

signals:

public slots:

protected:
    void create_widget_label(const QString &label);

    virtual void initialize_scene() = 0;

    void add_triangle(const Point *points, const QColor *colors);
    void add_line(const Point *points);

    const Project *const project;

    QVBoxLayout *layout;

private:
    friend class GuiOpenglWidget;

    void clear();

    int num_triangles;
    std::vector<GLfloat> triangle_vertices;
    std::vector<GLubyte> triangle_colors;
    std::vector<GLfloat> triangle_normals;

    int num_lines;
    std::vector<GLfloat> line_vertices;
};


} /* namespace os2cx */

#endif
