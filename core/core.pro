TARGET = os2cx
QT -= gui
CONFIG += c++14 console
CONFIG -= app_bundle

LIBS += -lgmp -lmpfr
LIBS += -ltet

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# valgrind doesn't support some of the float rounding modes that CGAL needs,
# causing CGAL to crash with an assertion on startup. Disable the check in
# profiling mode.
DEFINES += CGAL_DISABLE_ROUNDING_MATH_CHECK=ON

SOURCES += \
    calc.cpp \
    calculix_frd_read.cpp \
    calculix_inp_read.cpp \
    calculix_inp_write.cpp \
    calculix_run.cpp \
    measure.cpp \
    mesh.cpp \
    mesher_tetgen.cpp \
    mesh_index.cpp \
    mesh_type_info.cpp \
    openscad_extract.cpp \
    openscad_run.cpp \
    openscad_value.cpp \
    util.cpp \
    poly.cpp \
    beacon.cpp \
    main_backend.cpp \
    plc_nef.cpp \
    plc.cpp \
    plc_nef_to_plc.cpp \
    plc_index.cpp \
    result.cpp \
    units.cpp \
    project_run.cpp \
    mesher_naive_bricks.cpp \
    compute_attrs.cpp \
    attrs.cpp

HEADERS += \
    calc.hpp \
    calculix_frd_read.hpp \
    calculix_inp_read.hpp \
    calculix_inp_write.hpp \
    calculix_run.hpp \
    measure.hpp \
    mesh.hpp \
    mesher_tetgen.hpp \
    mesh_index.hpp \
    mesh_type_info.hpp \
    openscad_extract.hpp \
    openscad_run.hpp \
    openscad_value.hpp \
    project.hpp \
    result.hpp \
    util.hpp \
    poly.hpp \
    poly.internal.hpp \
    beacon.hpp \
    plc_nef.hpp \
    plc_nef.internal.hpp \
    plc.hpp \
    plc_nef_to_plc.hpp \
    plc_index.hpp \
    units.hpp \
    project_run.hpp \
    mesher_naive_bricks.hpp \
    compute_attrs.hpp \
    attrs.hpp

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
