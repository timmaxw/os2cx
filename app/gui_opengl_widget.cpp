#include "gui_opengl_widget.hpp"

#include <QMouseEvent>

#include "opengl.hpp"

namespace os2cx {

GuiOpenglWidget::GuiOpenglWidget(QWidget *parent, const Project *p) :
    QOpenGLWidget(parent), project(p)
{
    regenerate_scene();
}

void GuiOpenglWidget::regenerate_scene() {
    scene = project_to_opengl_scene(*project, scene_settings);
    update();
}

void GuiOpenglWidget::initializeGL() {
    initialize_opengl();
}

void GuiOpenglWidget::resizeGL(int w, int h) {
    resize_opengl(w, h);
}

void GuiOpenglWidget::paintGL() {
    scene->draw(draw_settings);
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
        draw_settings.drag(
            static_cast<float>(event->x() - mouse_last_x) / width(),
            static_cast<float>(event->y() - mouse_last_y) / width()
        );
        mouse_last_x = event->x();
        mouse_last_y = event->y();
        update();
    }
}

} /* namespace os2cx */
