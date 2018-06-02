#include "gui_scene_abstract.hpp"

#include <QPushButton>
#include <QMouseEvent>

namespace os2cx {

GuiSceneAbstract::GuiSceneAbstract(
    QWidget *parent,
    GuiSceneSettings *ss
) :
    QOpenGLWidget(parent),
    scene_settings(ss),
    num_triangles(0),
    num_lines(0)
{ }

void GuiSceneAbstract::add_triangle(
    const Point *points, const QColor *colors
) {
    PureVector normal = triangle_normal(points[0], points[1], points[2]);
    ++num_triangles;
    for (int i = 0; i < 3; ++i) {
        triangle_vertices.push_back(points[i].vector.x.val);
        triangle_vertices.push_back(points[i].vector.y.val);
        triangle_vertices.push_back(points[i].vector.z.val);
        triangle_colors.push_back(colors[i].red());
        triangle_colors.push_back(colors[i].green());
        triangle_colors.push_back(colors[i].blue());
        triangle_normals.push_back(normal.x.val);
        triangle_normals.push_back(normal.y.val);
        triangle_normals.push_back(normal.z.val);
    }
}

void GuiSceneAbstract::add_line(const Point *points) {
    ++num_lines;
    for (int i = 0; i < 2; ++i) {
        line_vertices.push_back(points[i].vector.x.val);
        line_vertices.push_back(points[i].vector.y.val);
        line_vertices.push_back(points[i].vector.z.val);
    }
}

void GuiSceneAbstract::initializeGL() {
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

    num_triangles = 0;
    triangle_vertices.clear();
    triangle_colors.clear();
    triangle_normals.clear();
    num_lines = 0;
    line_vertices.clear();
    initialize_scene();
}

static const float fov_slope_min = 0.5;

void GuiSceneAbstract::resizeGL(int viewport_width, int viewport_height) {
    glViewport(0, 0, viewport_width, viewport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#if 0
    float ratio_w_h = viewport_width / static_cast<float>(viewport_height);
    float fov_slope_y = (viewport_width > viewport_height)
        ? fov_slope_min
        : (fov_slope_min / ratio_w_h);
    float fov_angle_y = 2 * atan(fov_slope_y) / M_PI * 180;
    gluPerspective(
        fov_angle_y,
        ratio_w_h,
        0.1 /*clip close*/,
        200 /*clip far*/
    );
#endif
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

void GuiSceneAbstract::paintGL() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLfloat light_dir[4] = {1, 1, 1, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_dir);

    /* Position camera such that entire project is visible */
    float fov_slop_factor = 1.5;
    float dist = project()->approx_scale.val / fov_slope_min * fov_slop_factor;
    glTranslatef(0, 0, -dist);

    glRotatef(90 + scene_settings->pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-scene_settings->yaw, 0.0f, 0.0f, 1.0f);

    if (num_triangles != 0) {
        glEnable(GL_VERTEX_ARRAY);
        glEnable(GL_COLOR_ARRAY);
        glEnable(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, triangle_vertices.data());
        glColorPointer(3, GL_UNSIGNED_BYTE, 0, triangle_colors.data());
        glNormalPointer(GL_FLOAT, 0, triangle_normals.data());
        glDrawArrays(GL_TRIANGLES, 0, 3 * num_triangles);
        glDisable(GL_NORMAL_ARRAY);
        glDisable(GL_COLOR_ARRAY);
        glDisable(GL_VERTEX_ARRAY);
    }

    if (num_lines != 0) {
        glColor3f(0, 0, 0);
        glEnable(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, line_vertices.data());
        glDrawArrays(GL_LINES, 0, 2 * num_triangles);
        glDisable(GL_VERTEX_ARRAY);
    }

    glFlush();
}

void GuiSceneAbstract::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        mouse_last_x = event->x();
        mouse_last_y = event->y();
    }
}

void GuiSceneAbstract::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        /* This scaling factor is chosen such that dragging from the left side
        of the view to the right side will rotate the model 180 degrees. */
        double scale_factor = 180.0 / width();
        scene_settings->yaw += (event->x() - mouse_last_x) * scale_factor;
        scene_settings->pitch += (event->y() - mouse_last_y) * scale_factor;

        scene_settings->yaw -= 360 * floor(scene_settings->yaw / 360);
        if (scene_settings->pitch > 90) scene_settings->pitch = 90;
        if (scene_settings->pitch < -90) scene_settings->pitch = -90;

        update();

        mouse_last_x = event->x();
        mouse_last_y = event->y();
    }
}

} /* namespace os2cx */
