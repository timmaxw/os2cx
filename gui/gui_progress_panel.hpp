#ifndef OS2CX_GUI_PROGRESS_PANEL_HPP_
#define OS2CX_GUI_PROGRESS_PANEL_HPP_

#include <QProgressBar>
#include <QPushButton>
#include <QWidget>

#include "project.hpp"

namespace os2cx {

class GuiProgressPanel : public QWidget
{
    Q_OBJECT
public:
    GuiProgressPanel(QWidget *parent, const Project *project);

signals:
    void see_results();

public slots:
    void update();

private:
    const Project *project;
    QProgressBar *progress_bar;
    QPushButton *button_results;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_PROGRESS_PANEL_HPP_ */
