#ifndef OS2CX_GUI_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_OPENGL_WIDGET_HPP_

#include <QOpenGLWidget>
#include <QOpenGLFunctions_1_1>

#include "gui_mode_abstract.hpp"

namespace os2cx {

class GuiOpenglScene {
public:
    GuiOpenglScene();

    void add_triangle(const Point *points, const QColor *colors);
    void add_line(const Point *points);

    int num_triangles;
    std::vector<GLfloat> triangle_vertices;
    std::vector<GLubyte> triangle_colors;
    std::vector<GLfloat> triangle_normals;

    int num_lines;
    std::vector<GLfloat> line_vertices;
};

class GuiOpenglWidget :
    public QOpenGLWidget, public QOpenGLFunctions_1_1
{
    Q_OBJECT
public:
    GuiOpenglWidget(QWidget *parent, std::shared_ptr<const Project> project);

    void set_mode(GuiModeAbstract *mode);

public slots:
    void refresh_scene();

private:
    void initializeGL();
    void resizeGL(int viewport_width, int viewport_height);
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    const std::shared_ptr<const Project> project;
    GuiModeAbstract *mode;
    std::shared_ptr<const GuiOpenglScene> scene;
    int mouse_last_x, mouse_last_y;
    float yaw, pitch; /* in degrees */
};

} /* namespace os2cx */

#endif // OS2CX_GUI_OPENGL_WIDGET_HPP_
