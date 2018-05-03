#include "gui_opengl_widget.hpp"

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

} /* namespace os2cx */
