#include "gui_main_window.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GuiMainWindow w;
    w.show();

    return a.exec();
}
