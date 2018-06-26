#ifndef OS2CX_GUI_PROGRESS_PANEL_HPP_
#define OS2CX_GUI_PROGRESS_PANEL_HPP_

#include <QProgressBar>
#include <QPushButton>
#include <QWidget>

#include "gui_scene_abstract.hpp"

namespace os2cx {

class GuiSceneProgress : public GuiSceneAbstract
{
    Q_OBJECT
public:
    GuiSceneProgress(QWidget *parent, const Project *project);

signals:
    void see_results();

public slots:
    void update_progress();

private:
    void initialize_scene();

    QProgressBar *progress_bar;
    QPushButton *button_results;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_PROGRESS_PANEL_HPP_ */
