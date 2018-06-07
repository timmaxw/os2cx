TARGET = os2cx
QT -= gui
CONFIG += c++11 console
CONFIG -= app_bundle

LIBS += -lCGAL -lgmp -lmpfr
LIBS += -lGLU
LIBS += -ltet
LIBS += -ltiny-process-library

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
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
    polynomial.cpp \
    poly.cpp \
    poly_map.cpp \
    poly_map_index.cpp \
    beacon.cpp \
    main_backend.cpp \
    poly_map_internal_traveral.cpp \
    poly_map_internal_masks.cpp \
    plc_nef.cpp \
    plc.cpp \
    plc_nef_to_plc.cpp

HEADERS += \
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
    polynomial.hpp \
    poly.hpp \
    poly.internal.hpp \
    poly_map.hpp \
    poly_map.internal.hpp \
    poly_map_index.hpp \
    beacon.hpp \
    poly_map_internal_traversal.hpp \
    poly_map_internal_masks.hpp \
    plc_nef.hpp \
    plc_nef.internal.hpp \
    plc.hpp \
    plc_nef_to_plc.hpp

# The "gui" and "test" projects include all the same headers and sources as
# "core", minus "main.cpp". Prepare variables for them to use from this file.
defineReplace(core_path) {
    old = $$1
    new =
    for (name, old) {
        new += ../core/$${name}
    }
    return ($$new)
}
CORE_HEADERS = $$core_path($$HEADERS)
CORE_SOURCES = $$core_path($$SOURCES)
CORE_SOURCES -= ../core/main_backend.cpp
