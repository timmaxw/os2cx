#ifndef OS2CX_GUI_ABSTRACT_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_ABSTRACT_OPENGL_WIDGET_HPP_

#include <QOpenGLFunctions_1_1>
#include <QOpenGLWidget>

#include "project.hpp"

namespace os2cx {

/* GuiCameraSettings is the camera settings for the 3D scene. It's a separate
object from the scene because we want to preserve it when we destroy the old
scene and create a new one. */
class GuiCameraSettings {
public:
    GuiCameraSettings() : yaw(40), pitch(20) { }

    float yaw, pitch; /* in degrees */
};

class GuiSceneAbstract :
    public QOpenGLWidget, public QOpenGLFunctions_1_1
{
    Q_OBJECT
public:
    /* GuiSceneAbstract has a lot of subclasses and sub-subclasses; to make
    constructors less verbose, the constructor parameters are collected into the
    SceneParams class. */
    class SceneParams {
    public:
        QWidget *scene_parent;
        const Project *project;
        GuiCameraSettings *camera_settings;
    };
    GuiSceneAbstract(const SceneParams &params);

signals:

public slots:

protected:
    void add_triangle(const Point *points, const QColor *colors);
    void add_line(const Point *points);

    virtual void initialize_scene() = 0;

    const Project *const project;

private:
    void initializeGL();
    void resizeGL(int viewport_width, int viewport_height);
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    GuiCameraSettings *camera_settings;
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
