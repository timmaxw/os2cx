#include "gui_opengl_widget.hpp"

#include <QMouseEvent>
#include <QTime>

namespace os2cx {

GuiOpenglScene::GuiOpenglScene() :
    animate_mode(AnimateMode::None) { }

void GuiOpenglScene::add_triangle(
    const Point *points,
    const ComplexVector *deltas,
    const QColor *colors,
    bool xray
) {
    Primitives *p = xray ? &xray_primitives : &primitives;
    ++p->num_triangles;
    for (int i = 0; i < 3; ++i) {
        p->triangle_points.push_back(points[i]);
        p->triangle_deltas.push_back(deltas[i]);
        p->triangle_colors.push_back(colors[i].red());
        p->triangle_colors.push_back(colors[i].green());
        p->triangle_colors.push_back(colors[i].blue());
    }
}

void GuiOpenglScene::add_line(
    const Point *points, const ComplexVector *deltas, bool xray
) {
    Primitives *p = xray ? &xray_primitives : &primitives;
    ++p->num_lines;
    for (int i = 0; i < 2; ++i) {
        p->line_points.push_back(points[i]);
        p->line_deltas.push_back(deltas[i]);
    }
}

void GuiOpenglScene::add_vertex(
    Point point, ComplexVector delta, const QColor &color, bool xray
) {
    Primitives *p = xray ? &xray_primitives : &primitives;
    ++p->num_vertices;
    p->vertex_points.push_back(point);
    p->vertex_deltas.push_back(delta);
    p->vertex_colors.push_back(color.red());
    p->vertex_colors.push_back(color.green());
    p->vertex_colors.push_back(color.blue());
}

GuiOpenglScene::Primitives::Primitives() :
    num_triangles(0), num_lines(0), num_vertices(0) { }

GuiOpenglWidget::GuiOpenglWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    mode(nullptr),
    animate_timer(-1),
    look_at(Point::origin()),
    yaw(30),
    pitch(30),
    zoom(1)
{ }

void GuiOpenglWidget::set_mode(GuiModeAbstract *new_mode) {
    mode = new_mode;
    refresh_scene();
}

void GuiOpenglWidget::refresh_scene() {
    if (mode != nullptr) {
        scene = mode->make_scene();
    } else {
        scene = nullptr;
    }

    if (scene && scene->animate_mode != GuiOpenglScene::AnimateMode::None) {
        if (animate_timer == -1) {
            animate_timer = startTimer(33, Qt::PreciseTimer);
        }
    } else {
        if (animate_timer != -1) {
            killTimer(animate_timer);
            animate_timer = -1;
        }
    }

    update();
}


void GuiOpenglWidget::compute_fov() {
    static const float fov_slope_min = 0.5;

    /* Position camera such that entire project is visible */
    static const float fov_slop_factor = 1.5;
    approx_scale = mode ? mode->project->approx_scale : 1.0;
    camera_dist = approx_scale / fov_slope_min * fov_slop_factor / zoom;

    if (size().width() > size().height()) {
        fov_slope_y = fov_slope_min;
        fov_slope_x = fov_slope_y / size().height() * size().width();
    } else {
        fov_slope_x = fov_slope_min;
        fov_slope_y = fov_slope_x / size().width() * size().height();
    }
}

std::complex<double> GuiOpenglWidget::compute_animate_multiplier() {
    /* phase ramps from 0 to 1, then repeats. The fact that we're using
    currentMSecsSinceStartOfDay() means we'll will briefly glitch at midnight,
    but it's fine. */
    double temp;
    double phase = modf(
        (QTime::currentTime().msecsSinceStartOfDay() / 1000.0)
            * scene->animate_hz,
        &temp
    );
    switch (scene->animate_mode) {
        case GuiOpenglScene::AnimateMode::None:
            return 1.0;
        case GuiOpenglScene::AnimateMode::Sawtooth:
            /* Note, we pause briefly at the top of the ramp */
            return std::min(2 * phase, 1.0);
        case GuiOpenglScene::AnimateMode::Sine:
            return std::complex<double>(
                cos(2 * M_PI * phase),
                sin(2 * M_PI * phase)
            );
        default:
            assert(false);
    }
}

void GuiOpenglWidget::compute_points_and_normals(
    std::complex<double> multiplier,
    const GuiOpenglScene::Primitives &primitives,
    ComputedPrimitives *computed_primitives_out
) {
    const GuiOpenglScene::Primitives &p = primitives;
    ComputedPrimitives *cp = computed_primitives_out;

    cp->triangle_points.resize(9 * p.num_triangles);
    cp->triangle_normals.resize(9 * p.num_triangles);
    for (int i = 0; i < p.num_triangles; ++i) {
        Point points[3];
        for (int j = 0; j < 3; ++j) {
            points[j] = p.triangle_points[3 * i + j]
                + (multiplier * p.triangle_deltas[3 * i + j]).real();
        }
        Vector normal = triangle_normal(points[0], points[1], points[2]);
        for (int j = 0; j < 3; ++j) {
            cp->triangle_points[3 * (3 * i + j) + 0] = points[j].x;
            cp->triangle_points[3 * (3 * i + j) + 1] = points[j].y;
            cp->triangle_points[3 * (3 * i + j) + 2] = points[j].z;
            cp->triangle_normals[3 * (3 * i + j) + 0] = normal.x;
            cp->triangle_normals[3 * (3 * i + j) + 1] = normal.y;
            cp->triangle_normals[3 * (3 * i + j) + 2] = normal.z;
        }
    }

    cp->line_points.resize(6 * p.num_lines);
    for (int i = 0; i < p.num_lines; ++i) {
        for (int j = 0; j < 2; ++j) {
            Point point = p.line_points[2 * i + j]
                + (multiplier * p.line_deltas[2 * i + j]).real();
            cp->line_points[3 * (2 * i + j) + 0] = point.x;
            cp->line_points[3 * (2 * i + j) + 1] = point.y;
            cp->line_points[3 * (2 * i + j) + 2] = point.z;
        }
    }

    cp->vertex_points.resize(3 * p.num_vertices);
    for (int i = 0; i < p.num_vertices; ++i) {
        Point point = p.vertex_points[i]
            + (multiplier * p.vertex_deltas[i]).real();
        cp->vertex_points[3 * i + 0] = point.x;
        cp->vertex_points[3 * i + 1] = point.y;
        cp->vertex_points[3 * i + 2] = point.z;
    }
}

void GuiOpenglWidget::initializeGL() {
    bool res = initializeOpenGLFunctions();
    assert(res);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_ambient[4] = {0, 0, 0, 1};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    GLfloat light_diffuse[4] = {1, 1, 1, 1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    GLfloat light_specular[4] = {0, 0, 0, 1};
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    GLfloat light_model_ambient[4] = {0.3, 0.3, 0.3, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    GLfloat material_specular[4] = {0, 0, 0, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void GuiOpenglWidget::resizeGL(int viewport_width, int viewport_height) {
    (void)viewport_width;
    (void)viewport_height;
}

void GuiOpenglWidget::paint_primitives(
    const GuiOpenglScene::Primitives &primitives,
    const ComputedPrimitives &computed_primitives
) {
    const GuiOpenglScene::Primitives &p = primitives;
    const ComputedPrimitives &cp = computed_primitives;

    if (p.num_triangles != 0) {
        glEnable(GL_VERTEX_ARRAY);
        glEnable(GL_COLOR_ARRAY);
        glEnable(GL_NORMAL_ARRAY);
        glVertexPointer(
            3, GL_FLOAT, 0, cp.triangle_points.data());
        glColorPointer(
            3, GL_UNSIGNED_BYTE, 0, p.triangle_colors.data());
        glNormalPointer(
            GL_FLOAT, 0, cp.triangle_normals.data());
        glDrawArrays(GL_TRIANGLES, 0, 3 * p.num_triangles);
        glDisable(GL_NORMAL_ARRAY);
        glDisable(GL_COLOR_ARRAY);
        glDisable(GL_VERTEX_ARRAY);
    }

    if (p.num_lines != 0) {
        glColor3f(0, 0, 0);
        glEnable(GL_VERTEX_ARRAY);
        glVertexPointer(
            3, GL_FLOAT, 0, cp.line_points.data());
        glDrawArrays(GL_LINES, 0, 2 * p.num_lines);
        glDisable(GL_VERTEX_ARRAY);
    }

    if (p.num_vertices != 0) {
        glDisable(GL_LIGHTING);
        glEnable(GL_VERTEX_ARRAY);
        glVertexPointer(
            3, GL_FLOAT, 0, cp.vertex_points.data());

        /* Draw a 10-pixel point in black, which will form a black border */
        glColor3f(0, 0, 0);
        glPointSize(10);
        glDrawArrays(GL_POINTS, 0, p.num_vertices);

        /* Draw an 8-pixel point in the intended color */
        glEnable(GL_COLOR_ARRAY);
        glColorPointer(
            3, GL_UNSIGNED_BYTE, 0, p.vertex_colors.data());
        glPointSize(8);
        glDrawArrays(GL_POINTS, 0, p.num_vertices);

        glDisable(GL_COLOR_ARRAY);
        glDisable(GL_VERTEX_ARRAY);
        glEnable(GL_LIGHTING);
    }
}

static const uint8_t depth_buffer_stipple_pattern[4 * 32] = {
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
    0b11111111, 0b11111111, 0b11111111, 0b11111111,
    0b01010101, 0b01010101, 0b01010101, 0b01010101,
};

void GuiOpenglWidget::paint_stipple_to_depth_buffer() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    /* Disable both matrices, so that x=+1/-1 map directly to the left/right
    sides of the viewport, and so on */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDepthFunc(GL_ALWAYS);

    /* We only want to affect the depth buffer, not the color buffer */
    glColorMask(false, false, false, false);
    // glColor3f(1.0, 0.0, 0.0);

    /* We only want to affect the depth buffer in a stipple pattern, not every
    single pixel */
    glEnable(GL_POLYGON_STIPPLE);
    glPolygonStipple(depth_buffer_stipple_pattern);

    /* Draw a rectangle that covers the entire viewpoint with depth 1.0 */
    glBegin(GL_QUADS);
    glVertex3f(-1, -1, 1.0);
    glVertex3f(+1, -1, 1.0);
    glVertex3f(+1, +1, 1.0);
    glVertex3f(-1, +1, 1.0);
    glEnd();

    /* Reset to the previous state */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();
}

void GuiOpenglWidget::paintGL() {
    glViewport(0, 0, size().width(), size().height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    compute_fov();
    float z_near = approx_scale / 1e3;
    float z_far = camera_dist + 2*approx_scale;

    glFrustum(
        -fov_slope_x * z_near, fov_slope_x * z_near,
        -fov_slope_y * z_near, fov_slope_y * z_near,
        z_near, z_far);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLfloat light_dir[4] = {0.0, 1.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_dir);

    glTranslatef(0, 0, -camera_dist);
    glRotatef(-90 + pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-yaw, 0.0f, 0.0f, 1.0f);
    glTranslatef(look_at.x, look_at.y, look_at.z);

    if (scene != nullptr) {
        compute_points_and_normals(
            compute_animate_multiplier(),
            scene->primitives,
            &computed_primitives);
        compute_points_and_normals(
            compute_animate_multiplier(),
            scene->xray_primitives,
            &computed_xray_primitives);

        /* First, draw non-xray primitives normally. */
        glCullFace(GL_BACK);
        paint_primitives(scene->primitives, computed_primitives);

        /* On alternate pixels, reset the depth buffer to max depth; this
        ensures that xray primitives will be drawn over non-xray primitives on
        those pixels. */
        paint_stipple_to_depth_buffer();

        /* Now draw x-ray primitives. x-ray primitives can be seen from any
        direction, so disable face culling. */
        glDisable(GL_CULL_FACE);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
        paint_primitives(scene->xray_primitives, computed_xray_primitives);
        glEnable(GL_CULL_FACE);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
    }

    glFlush();
}

void GuiOpenglWidget::timerEvent(QTimerEvent *) {
    update();
}

void GuiOpenglWidget::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() != 0) {
        mouse_last_x = event->x();
        mouse_last_y = event->y();
    }
}

void GuiOpenglWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        /* This scaling factor is chosen such that dragging from the left side
        of the view to the right side will rotate the model 180 degrees. */
        double scale_factor = 180.0 / width();
        yaw -= (event->x() - mouse_last_x) * scale_factor;
        pitch += (event->y() - mouse_last_y) * scale_factor;

        yaw -= 360 * floor(yaw / 360);
        if (pitch > 90) pitch = 90;
        if (pitch < -90) pitch = -90;

        update();
    }

    if (event->buttons() & Qt::RightButton) {
        compute_fov();

        Length move_h = static_cast<double>(event->x() - mouse_last_x)
            / width() * 2 * camera_dist * fov_slope_x;
        Vector dir_h(cos(yaw / 180 * M_PI), sin(yaw / 180 * M_PI), 0);
        look_at += move_h * dir_h;

        Length move_v = -static_cast<double>(event->y() - mouse_last_y)
            / height() * 2 * camera_dist * fov_slope_y;
        Vector dir_v(
            -sin(yaw / 180 * M_PI) * sin(pitch / 180 * M_PI),
            cos(yaw / 180 * M_PI) * sin(pitch / 180 * M_PI),
            cos(pitch / 180 * M_PI));
        look_at += move_v * dir_v;

        /* Don't let the camera go too far from the model */
        Vector vector_from_origin = look_at - Point::origin();
        Length distance_from_origin = vector_from_origin.magnitude();
        if (distance_from_origin > approx_scale) {
            vector_from_origin *= (approx_scale / distance_from_origin);
            look_at = Point::origin() + vector_from_origin;
        }

        update();
    }

    if (event->buttons() != 0) {
        mouse_last_x = event->x();
        mouse_last_y = event->y();
    }
}

void GuiOpenglWidget::wheelEvent(QWheelEvent *event) {
    zoom *= pow(2, -event->delta() / (360.0 * 8.0));
    if (zoom < 1) zoom = 1;
    if (zoom > 100) zoom = 100;
    update();
}

} /* namespace os2cx */
