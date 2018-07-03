#ifndef OS2CX_GUI_MODE_ABSTRACT_HPP_
#define OS2CX_GUI_MODE_ABSTRACT_HPP_

#include <QOpenGLFunctions_1_1>
#include <QVBoxLayout>
#include <QWidget>

#include "project.hpp"

namespace os2cx {

class GuiOpenglTriangles;

class GuiModeAbstract : public QWidget
{
    Q_OBJECT
public:
    GuiModeAbstract(QWidget *parent, const Project *project);

    virtual std::shared_ptr<const GuiOpenglTriangles> make_triangles() = 0;

signals:
    void refresh_scene();

public slots:

protected:
    void create_widget_label(const QString &label);

    const Project *const project;

    QVBoxLayout *layout;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_ABSTRACT_HPP_ */
