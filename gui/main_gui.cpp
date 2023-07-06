#include <QApplication>
#include <QFileDialog>

#include "gui_main_window.hpp"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setWindowIcon(QIcon(":/OpenSCAD2CalculiX.png"));

    std::string scad_path;
    if (argc == 1) {
        QString scad_path_qstring = QFileDialog::getOpenFileName(
            nullptr,
            application.tr("Choose OpenSCAD file to simulate"),
            QString(),
            application.tr("OpenSCAD files (*.scad)"));
        if (scad_path_qstring.isNull()) {
            /* The user clicked 'cancel' */
            return 0;
        }
        scad_path = scad_path_qstring.toStdString();
    } else if (argc == 2) {
        scad_path = argv[1];
    } else {
        std::cerr << "usage: openscad2calculix [input.scad]" << std::endl;
        return 1;
    }

    os2cx::GuiMainWindow main_window(scad_path);
    main_window.show();

    return application.exec();
}
