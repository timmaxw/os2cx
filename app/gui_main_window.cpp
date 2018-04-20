#include "gui_main_window.hpp"

#include "ui_gui_main_window.h"

GuiMainWindow::GuiMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GuiMainWindow)
{
    ui->setupUi(this);
}

GuiMainWindow::~GuiMainWindow() {
    delete ui;
}
