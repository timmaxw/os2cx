include(../core/core.pro)
INCLUDEPATH += ../core

TARGET = OpenSCAD2CalculiX
TEMPLATE = app
QT += core gui widgets
CONFIG += app_bundle
CONFIG -= console

HEADERS = $$CORE_HEADERS \
    gui_project_runner.hpp \
    gui_main_window.hpp \
    gui_opengl_widget.hpp \
    gui_color_scale.hpp \
    gui_mode_abstract.hpp \
    gui_mode_progress.hpp \
    gui_mode_result.hpp \
    gui_combo_box_modes.hpp \
    gui_opengl_mesh.hpp \
    gui_opengl_poly3.hpp \
    gui_mode_inspect.hpp

SOURCES = $$CORE_SOURCES \
    gui_project_runner.cpp \
    main_gui.cpp \
    gui_main_window.cpp \
    gui_opengl_widget.cpp \
    gui_color_scale.cpp \
    gui_mode_abstract.cpp \
    gui_mode_progress.cpp \
    gui_mode_result.cpp \
    gui_combo_box_modes.cpp \
    gui_opengl_mesh.cpp \
    gui_opengl_poly3.cpp \
    gui_mode_inspect.cpp
