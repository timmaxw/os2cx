#ifndef OS2CX_GUI_MAIN_WINDOW_HPP_
#define OX2CX_GUI_MAIN_WINDOW_HPP_

#include <QComboBox>
#include <QMainWindow>
#include <QSplitter>
#include <QVBoxLayout>

#include "gui_combo_box_modes.hpp"
#include "gui_opengl_widget.hpp"
#include "gui_project_runner.hpp"
#include "gui_mode_abstract.hpp"
#include "project.hpp"

namespace os2cx {

class GuiMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GuiMainWindow(const std::string &scad_path);

public slots:
    void menu_file_open();
    void refresh_combo_box_modes();
    void set_current_mode(GuiModeAbstract *new_mode);

private:
    QSize sizeHint() const;

    GuiProjectRunner *project_runner;

    QSplitter *splitter;
    QWidget *left_panel;
    QVBoxLayout *left_panel_layout;
    GuiOpenglWidget *right_panel;

    GuiComboBoxModes *combo_box_modes;
    GuiModeAbstract *current_mode;
    QString first_result_mode_name;
};

} /* namespace os2cx */

#endif
