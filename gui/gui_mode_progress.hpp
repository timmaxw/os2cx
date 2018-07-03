#ifndef OS2CX_GUI_MODE_PROGRESS_HPP_
#define OS2CX_GUI_MODE_PROGRESS_HPP_

#include <QProgressBar>
#include <QPushButton>
#include <QWidget>

#include "gui_mode_abstract.hpp"

namespace os2cx {

class GuiModeProgress : public GuiModeAbstract
{
    Q_OBJECT
public:
    GuiModeProgress(QWidget *parent, const Project *project);

signals:
    void see_results();

public slots:
    void update_progress();

private:
    std::shared_ptr<const GuiOpenglScene> make_scene();

    QProgressBar *progress_bar;
    QPushButton *button_results;
};

} /* namespace os2cx */

#endif /* OS2CX_GUI_MODE_PROGRESS_HPP_ */
