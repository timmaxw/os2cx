#ifndef GUI_MAIN_WINDOW_HPP
#define GUI_MAIN_WINDOW_HPP

#include <QMainWindow>

namespace Ui {
class GuiMainWindow;
} /* namespace Ui */

class GuiMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GuiMainWindow(QWidget *parent = 0);
    ~GuiMainWindow();

private:
    Ui::GuiMainWindow *ui;
};

#endif // GUI_MAIN_WINDOW_HPP
