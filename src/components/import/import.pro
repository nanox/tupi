# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/components/import
# Target is a library:  

INSTALLS += target
target.path = /lib/

macx {
    CONFIG += staticlib warn_on
}

HEADERS += tuppaletteimporter.h
SOURCES += tuppaletteimporter.cpp

*:!macx{
    CONFIG += dll warn_on
}

TEMPLATE = lib 
TARGET = tupimport

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)
include(../components_config.pri)
