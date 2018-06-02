#ifndef OS2CX_GUI_ABSTRACT_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_ABSTRACT_OPENGL_WIDGET_HPP_

#include <QOpenGLFunctions_1_1>
#include <QOpenGLWidget>

#include "project.hpp"

namespace os2cx {

class GuiSceneSettings {
public:
    GuiSceneSettings(const Project *p) : project(p), yaw(40), pitch(20) { }

    const Project *project;
    float yaw, pitch; /* in degrees */
};

class GuiSceneAbstract :
    public QOpenGLWidget, public QOpenGLFunctions_1_1
{
    Q_OBJECT
public:
    GuiSceneAbstract(QWidget *parent, GuiSceneSettings *settings);

signals:

public slots:

protected:
    void add_triangle(const Point *points, const QColor *colors);
    void add_line(const Point *points);

    virtual void initialize_scene() = 0;

    const Project *project() { return scene_settings->project; }

private:
    void initializeGL();
    void resizeGL(int viewport_width, int viewport_height);
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    GuiSceneSettings *scene_settings;
    Length approx_scale;
    int mouse_last_x, mouse_last_y;

    int num_triangles;
    std::vector<GLfloat> triangle_vertices;
    std::vector<GLubyte> triangle_colors;
    std::vector<GLfloat> triangle_normals;

    int num_lines;
    std::vector<GLfloat> line_vertices;
};

} /* namespace os2cx */

#endif
