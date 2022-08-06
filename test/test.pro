include(../core/core.pro)
INCLUDEPATH += ../core

TARGET = test
CONFIG -= app_bundle
CONFIG += console
LIBS += -lgtest -lgtest_main

HEADERS = $$CORE_HEADERS

SOURCES = $$CORE_SOURCES \
    attrs_test.cpp \
    calculix_read_test.cpp \
    mesh_index_test.cpp \
    openscad_extract_test.cpp \
    openscad_run_test.cpp \
    openscad_value_test.cpp \
    poly_test.cpp \
    beacon_test.cpp \
    plc_nef_test.cpp \
    plc_test.cpp \
    units_test.cpp \
    mesh_test.cpp \
    mesher_naive_bricks_test.cpp \
    mesh_type_info_test.cpp

DISTFILES += \
    max_element_size_test.scad \
    mesher_element_type_test.scad \
    multi_material_test.scad \
    slice_test.scad \
    steady_state_dynamics_test.scad \
    surface_internal_test.scad

