TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    src/ShaderProgram.cpp \
    src/Shader.cpp \
    src/stb_image.cpp \
    src/main.cpp

OTHER_FILES += \
    resources.txt

HEADERS += \
    include/ShaderProgram.h \
    include/Shader.h \
    include/FileLoader.h \
    include/stb_image.h

