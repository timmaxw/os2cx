#include "gui_opengl_widget.hpp"

#include <QMouseEvent>

namespace os2cx {

GuiOpenglScene::GuiOpenglScene() :
    num_triangles(0), num_lines(0)
{ }

void GuiOpenglScene::add_triangle(
    const Point *points, const QColor *colors
) {
    Vector normal = triangle_normal(points[0], points[1], points[2]);
    ++num_triangles;
    for (int i = 0; i < 3; ++i) {
        triangle_vertices.push_back(points[i].x);
        triangle_vertices.push_back(points[i].y);
        triangle_vertices.push_back(points[i].z);
        triangle_colors.push_back(colors[i].red());
        triangle_colors.push_back(colors[i].green());
        triangle_colors.push_back(colors[i].blue());
        triangle_normals.push_back(normal.x);
        triangle_normals.push_back(normal.y);
        triangle_normals.push_back(normal.z);
    }
}

void GuiOpenglScene::add_line(const Point *points) {
    ++num_lines;
    for (int i = 0; i < 2; ++i) {
        line_vertices.push_back(points[i].x);
        line_vertices.push_back(points[i].y);
        line_vertices.push_back(points[i].z);
    }
}

GuiOpenglWidget::GuiOpenglWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    mode(nullptr),
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

void GuiOpenglWidget::initializeGL() {
    bool res = initializeOpenGLFunctions();
    assert(res);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Grey Background
    glClearDepth(1.0f); // Depth Buffer Setup
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
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

    /* TODO: Back-face culling */
}

void GuiOpenglWidget::resizeGL(int viewport_width, int viewport_height) {
    (void)viewport_width;
    (void)viewport_height;
}

void GuiOpenglWidget::paintGL() {
    glViewport(0, 0, size().width(), size().height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    compute_fov();
    float z_near = approx_scale / 1e3;
    float z_far = camera_dist + approx_scale;

    glFrustum(
        -fov_slope_x * z_near, fov_slope_x * z_near,
        -fov_slope_y * z_near, fov_slope_y * z_near,
        z_near, z_far);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLfloat light_dir[4] = {1, 1, 1, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_dir);

    glTranslatef(0, 0, -camera_dist);
    glRotatef(-90 + pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-yaw, 0.0f, 0.0f, 1.0f);
    glTranslatef(look_at.x, look_at.y, look_at.z);

    if (scene != nullptr) {
        if (scene->num_triangles != 0) {
            glEnable(GL_VERTEX_ARRAY);
            glEnable(GL_COLOR_ARRAY);
            glEnable(GL_NORMAL_ARRAY);
            glVertexPointer(
                3, GL_FLOAT, 0, scene->triangle_vertices.data());
            glColorPointer(
                3, GL_UNSIGNED_BYTE, 0, scene->triangle_colors.data());
            glNormalPointer(GL_FLOAT, 0, scene->triangle_normals.data());
            glDrawArrays(GL_TRIANGLES, 0, 3 * scene->num_triangles);
            glDisable(GL_NORMAL_ARRAY);
            glDisable(GL_COLOR_ARRAY);
            glDisable(GL_VERTEX_ARRAY);
        }

        if (scene->num_lines != 0) {
            glColor3f(0, 0, 0);
            glEnable(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, scene->line_vertices.data());
            glDrawArrays(GL_LINES, 0, 2 * scene->num_lines);
            glDisable(GL_VERTEX_ARRAY);
        }
    }

    glFlush();
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
