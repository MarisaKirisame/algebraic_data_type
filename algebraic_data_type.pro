TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

QMAKE_CXXFLAGS += -std=c++1z
INCLUDEPATH += /home/marisa/GitSource/Idionne/hana/include

HEADERS += \
    algebraic_data_type.hpp
