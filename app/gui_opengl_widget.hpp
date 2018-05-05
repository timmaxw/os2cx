#ifndef OS2CX_GUI_OPENGL_WIDGET_HPP_
#define OS2CX_GUI_OPENGL_WIDGET_HPP_

#include <QOpenGLWidget>

#include "opengl.hpp"
#include "project.hpp"

namespace os2cx {

class GuiOpenglWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit GuiOpenglWidget(QWidget *parent, const Project *project);

    void regenerate_scene();

    OpenglSceneSettings scene_settings;
    OpenglDrawSettings draw_settings;

signals:

public slots:

private:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    const Project *project;
    std::unique_ptr<OpenglScene> scene;
    int mouse_last_x, mouse_last_y;
};

} /* namespace os2cx */

#endif
