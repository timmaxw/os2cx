include(../app/app.pro)

INCLUDEPATH += ../app

TARGET = test
CONFIG -= app_bundle
CONFIG += console
LIBS += -lgtest -lgtest_main

defineReplace(app_path) {
    old = $$1
    new =
    for (name, old) {
        new += ../app/$${name}
    }
    return ($$new)
}
APP_HEADERS = $$app_path($$HEADERS)
APP_SOURCES = $$app_path($$SOURCES)
APP_FORMS = $$app_path($$FORMS)
APP_SOURCES -= ../app/main.cpp

HEADERS = $$APP_HEADERS
SOURCES = $$APP_SOURCES \
    attrs_test.cpp \
    calculix_read_test.cpp \
    mesh_index_test.cpp \
    mesher_external_test.cpp \
    openscad_extract_test.cpp \
    openscad_run_test.cpp \
    openscad_value_test.cpp \
    polynomial_test.cpp \
    poly_map_test.cpp \
    poly_test.cpp
FORMS = $$APP_FORMS
