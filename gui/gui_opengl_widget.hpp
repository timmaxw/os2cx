#ifndef OS2CX_GUI_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_OPENGL_WIDGET_HPP_

#include <QOpenGLWidget>
#include <QOpenGLFunctions_1_1>

#include "gui_mode_abstract.hpp"

namespace os2cx {

class GuiOpenglScene {
public:
    GuiOpenglScene();

    void add_triangle(const Point *points, const Vector *deltas, const QColor *colors);
    void add_line(const Point *points, const Vector *deltas);

    int num_triangles;
    std::vector<Point> triangle_points;
    std::vector<Vector> triangle_deltas;
    std::vector<GLubyte> triangle_colors;

    int num_lines;
    std::vector<Point> line_points;
    std::vector<Vector> line_deltas;

    enum class AnimateMode {
        None,
        Sawtooth,
        Sine
    };
    AnimateMode animate_mode;
    double animate_hz;
};

class GuiOpenglWidget :
    public QOpenGLWidget, public QOpenGLFunctions_1_1
{
    Q_OBJECT
public:
    GuiOpenglWidget(QWidget *parent);

    void set_mode(GuiModeAbstract *mode);

public slots:
    void refresh_scene();

private:
    void compute_fov();
    double compute_animate_multiplier();
    void compute_points_and_normals(double multiplier);

    void initializeGL();
    void resizeGL(int viewport_width, int viewport_height);
    void paintGL();

    void timerEvent(QTimerEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    GuiModeAbstract *mode;
    std::shared_ptr<const GuiOpenglScene> scene;
    int mouse_last_x, mouse_last_y;
    int animate_timer;

    /* These variables are all computed by setup_camera() */
    double approx_scale;
    float fov_slope_x, fov_slope_y;
    Length camera_dist;

    Point look_at;
    float yaw, pitch; /* in degrees */
    float zoom;

    std::vector<GLfloat> triangle_computed_points, triangle_computed_normals;
    std::vector<GLfloat> line_computed_points;
};

} /* namespace os2cx */

#endif // OS2CX_GUI_OPENGL_WIDGET_HPP_
