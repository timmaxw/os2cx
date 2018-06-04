#include <QApplication>

#include "gui_main_window.hpp"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    os2cx::GuiMainWindow main_window("example.scad");
    main_window.show();

    return application.exec();
}
