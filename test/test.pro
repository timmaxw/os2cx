include(../app/app.pro)

defineReplace(app_path) {
    old = $$1
    new =
    for (name, old) {
        new += ../app/$${name}
    }
    return ($$new)
}
HEADERS = $$app_path($$HEADERS)
SOURCES = $$app_path($$SOURCES)
FORMS = $$app_path($$FORMS)

SOURCES -= ../app/main.cpp

INCLUDEPATH += ../app

TARGET = test
CONFIG -= app_bundle
CONFIG += console
LIBS += -lgtest -lgtest_main

SOURCES += \
    attrs_test.cpp \
    calculix_read_test.cpp \
    mesh_index_test.cpp \
    mesher_external_test.cpp \
    openscad_extract_test.cpp \
    openscad_run_test.cpp \
    openscad_value_test.cpp \
    region_test.cpp \
    region_map_test.cpp
