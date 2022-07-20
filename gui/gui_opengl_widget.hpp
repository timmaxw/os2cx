#ifndef OS2CX_GUI_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_OPENGL_WIDGET_HPP_

#include <QOpenGLWidget>
#include <QOpenGLFunctions_1_1>

#include "gui_mode_abstract.hpp"

namespace os2cx {

class GuiOpenglScene {
public:
    GuiOpenglScene();

    void add_triangle(
        const Point *points,
        const ComplexVector *deltas,
        const QColor *colors,
        bool xray);
    void add_line(
        const Point *points, const ComplexVector *deltas, bool xray);
    void add_vertex(
        Point point, ComplexVector delta, const QColor &color, bool xray);

    enum class AnimateMode {
        None,
        Sawtooth,
        Sine
    };
    AnimateMode animate_mode;
    double animate_hz;

private:
    friend class GuiOpenglWidget;

    struct Primitives {
        Primitives();

        int num_triangles;
        std::vector<Point> triangle_points;
        std::vector<ComplexVector> triangle_deltas;
        std::vector<GLubyte> triangle_colors;

        int num_lines;
        std::vector<Point> line_points;
        std::vector<ComplexVector> line_deltas;

        int num_vertices;
        std::vector<Point> vertex_points;
        std::vector<ComplexVector> vertex_deltas;
        std::vector<GLubyte> vertex_colors;
    };
    Primitives primitives, xray_primitives;
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
    struct ComputedPrimitives {
        std::vector<GLfloat> triangle_points, triangle_normals;
        std::vector<GLfloat> line_points;
        std::vector<GLfloat> vertex_points;
    };

    void compute_fov();
    std::complex<double> compute_animate_multiplier();
    void compute_points_and_normals(
        std::complex<double> multiplier,
        const GuiOpenglScene::Primitives &primitives,
        ComputedPrimitives *computed_primitives_out);

    void initializeGL();
    void resizeGL(int viewport_width, int viewport_height);
    void paint_primitives(
        const GuiOpenglScene::Primitives &primitives,
        const ComputedPrimitives &computed_primitives,
        bool xray);
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

    ComputedPrimitives computed_primitives, computed_xray_primitives;
};

} /* namespace os2cx */

#endif // OS2CX_GUI_OPENGL_WIDGET_HPP_
