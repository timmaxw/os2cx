include(../core/core.pro)
INCLUDEPATH += ../core

TARGET = OpenSCAD2CalculiX
TEMPLATE = app
QT += core gui widgets
CONFIG += app_bundle
CONFIG -= console

HEADERS = $$CORE_HEADERS \
    gui_project_runner.hpp \
    gui_focus_combo_box.hpp \
    gui_scene_abstract.hpp \
    gui_scene_mesh.hpp \
    gui_scene_poly3.hpp \
    gui_main_window.hpp

SOURCES = $$CORE_SOURCES \
    gui_project_runner.cpp \
    gui_focus_combo_box.cpp \
    gui_scene_abstract.cpp \
    gui_scene_mesh.cpp \
    gui_scene_poly3.cpp \
    main_gui.cpp \
    gui_main_window.cpp
