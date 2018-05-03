#ifndef GUI_OPENGL_WIDGET_HPP
#define GUI_OPENGL_WIDGET_HPP

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

    const Project *project;
    std::unique_ptr<OpenglScene> scene;
};

} /* namespace os2cx */

#endif // GUI_OPENGL_WIDGET_HPP
