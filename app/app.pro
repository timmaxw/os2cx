#-------------------------------------------------
#
# Project created by QtCreator 2018-04-17T21:17:44
#
#-------------------------------------------------

QT += core gui widgets

TARGET = app
TEMPLATE = app

LIBS += -lCGAL -lgmp -lmpfr
LIBS += -lGLU
LIBS += -ltet
LIBS += -ltiny-process-library

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        gui_main_window.cpp \
    attrs.cpp \
    calc.cpp \
    calculix_frd_read.cpp \
    calculix_inp_read.cpp \
    calculix_inp_write.cpp \
    calculix_run.cpp \
    mesh.cpp \
    mesher_external.cpp \
    mesher_tetgen.cpp \
    mesh_index.cpp \
    mesh_type_info.cpp \
    openscad_extract.cpp \
    openscad_run.cpp \
    openscad_value.cpp \
    project.cpp \
    util.cpp \
    gui_project_runner.cpp \
    gui_focus_combo_box.cpp \
    polynomial.cpp \
    poly.cpp \
    poly_map.cpp \
    poly_map_index.cpp \
    gui_scene_abstract.cpp \
    gui_scene_mesh.cpp \
    gui_scene_poly3.cpp

HEADERS += \
        gui_main_window.hpp \
    attrs.hpp \
    calc.hpp \
    calculix_frd_read.hpp \
    calculix_inp_read.hpp \
    calculix_inp_write.hpp \
    calculix_run.hpp \
    mesh.hpp \
    mesher_external.hpp \
    mesher_tetgen.hpp \
    mesh_index.hpp \
    mesh_type_info.hpp \
    openscad_extract.hpp \
    openscad_run.hpp \
    openscad_value.hpp \
    project.hpp \
    result.hpp \
    util.hpp \
    gui_project_runner.hpp \
    gui_focus_combo_box.hpp \
    polynomial.hpp \
    poly.hpp \
    poly.internal.hpp \
    poly_map.hpp \
    poly_map.internal.hpp \
    poly_map_index.hpp \
    gui_scene_abstract.hpp \
    gui_scene_mesh.hpp \
    gui_scene_poly3.hpp

FORMS +=
