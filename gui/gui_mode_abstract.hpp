#ifndef OS2CX_GUI_MODE_ABSTRACT_HPP_
#define OS2CX_GUI_MODE_ABSTRACT_HPP_

#include <QLabel>
#include <QOpenGLFunctions_1_1>
#include <QVBoxLayout>
#include <QWidget>

#include "project.hpp"

namespace os2cx {

class GuiOpenglScene;

class GuiModeAbstract : public QWidget
{
    Q_OBJECT
public:
    GuiModeAbstract(QWidget *parent, std::shared_ptr<const Project> project);

    virtual std::shared_ptr<const GuiOpenglScene> make_scene() = 0;

    const std::shared_ptr<const Project> project;

signals:
    void refresh_scene();

public slots:

protected:
    QLabel *create_widget_label(const QString &label);

    QVBoxLayout *layout;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_ABSTRACT_HPP_ */
