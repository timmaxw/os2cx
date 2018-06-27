#include "gui_opengl_widget.hpp"

#include <QMouseEvent>

namespace os2cx {

GuiOpenglWidget::GuiOpenglWidget(QWidget *parent, const Project *project_) :
    QOpenGLWidget(parent),
    project(project_),
    scene(nullptr),
    yaw(20),
    pitch(40)
{ }

void GuiOpenglWidget::set_scene(GuiSceneAbstract *new_scene) {
    if (scene != nullptr) {
        scene->clear();
    }
    if (new_scene != nullptr) {
        new_scene->clear();
        new_scene->initialize_scene();
    }
    scene = new_scene;
    update();
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

    if (scene != nullptr) {
        scene->clear();
        scene->initialize_scene();
    }
}

static const float fov_slope_min = 0.5;

void GuiOpenglWidget::resizeGL(int viewport_width, int viewport_height) {
    glViewport(0, 0, viewport_width, viewport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float slope_x, slope_y;
    if (viewport_width > viewport_height) {
        slope_y = fov_slope_min;
        slope_x = slope_y / viewport_height * viewport_width;
    } else {
        slope_x = fov_slope_min;
        slope_y = slope_x / viewport_width * viewport_height;
    }
    float z_near = 0.1, z_far = 200;
    glFrustum(
        -slope_x * z_near, slope_x * z_near,
        -slope_y * z_near, slope_y * z_near,
        z_near, z_far);
}

void GuiOpenglWidget::paintGL() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLfloat light_dir[4] = {1, 1, 1, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_dir);

    /* Position camera such that entire project is visible */
    float fov_slop_factor = 1.5;
    float dist = project->approx_scale.val / fov_slope_min * fov_slop_factor;
    glTranslatef(0, 0, -dist);

    glRotatef(90 + pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-yaw, 0.0f, 0.0f, 1.0f);

    if (scene != nullptr) {
        if (scene->num_triangles != 0) {
            glEnable(GL_VERTEX_ARRAY);
            glEnable(GL_COLOR_ARRAY);
            glEnable(GL_NORMAL_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, scene->triangle_vertices.data());
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
            glDrawArrays(GL_LINES, 0, 2 * scene->num_triangles);
            glDisable(GL_VERTEX_ARRAY);
        }
    }

    glFlush();
}


void GuiOpenglWidget::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        mouse_last_x = event->x();
        mouse_last_y = event->y();
    }
}

void GuiOpenglWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        /* This scaling factor is chosen such that dragging from the left side
        of the view to the right side will rotate the model 180 degrees. */
        double scale_factor = 180.0 / width();
        yaw += (event->x() - mouse_last_x) * scale_factor;
        pitch += (event->y() - mouse_last_y) * scale_factor;

        yaw -= 360 * floor(yaw / 360);
        if (pitch > 90) pitch = 90;
        if (pitch < -90) pitch = -90;

        update();

        mouse_last_x = event->x();
        mouse_last_y = event->y();
    }
}


} /* namespace os2cx */
