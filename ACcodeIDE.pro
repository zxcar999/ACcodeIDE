#-------------------------------------------------
# Project created by QtCreator
#-------------------------------------------------

QT       += core widgets gui

TARGET = ACcodeIDE
TEMPLATE = app

# 定义源文件和头文件（注意：QCodeEditor 的文件都在 internal 子目录下）
SOURCES += \
    main.cpp \
    accodeide.cpp \
    src/internal/QCodeEditor.cpp \
    src/internal/QCXXHighlighter.cpp \
    src/internal/QLanguage.cpp \
    src/internal/QLineNumberArea.cpp \
    src/internal/QStyleSyntaxHighlighter.cpp \
    src/internal/QSyntaxStyle.cpp \
    src/internal/QFramedTextAttribute.cpp \

HEADERS += \
    accodeide.h \
    include/internal/QCodeEditor.hpp \
    include/internal/QCXXHighlighter.hpp \
    include/internal/QLanguage.hpp \
    include/internal/QLineNumberArea.hpp \
    include/internal/QStyleSyntaxHighlighter.hpp \
    include/internal/QSyntaxStyle.hpp \
    include/internal/QFramedTextAttribute.hpp

INCLUDEPATH += $$PWD/include $$PWD/include/internal

FORMS += accodeide.ui

# 添加资源文件
RESOURCES += \
    resources/qcodeeditor_resources.qrc

# 如果需要翻译文件
TRANSLATIONS += ACcodeIDE_zh_CN.ts
y

DISTFILES += \
    favicon.ico \
    favicon.rc \
    resources/languages/cpp.xml \
    resources/languages/glsl.xml \
    resources/languages/lua.xml \
    resources/languages/python.xml

# 版本号（必须）
VERSION = 1.0.0.0

RC_FILE = favicon.rc


# 公司名称
QMAKE_TARGET_COMPANY = "zxcar999"

# 版权信息（最关键的一行）
QMAKE_TARGET_COPYRIGHT = "Copyright 2026 zxcar999."

# 产品名称
QMAKE_TARGET_PRODUCT = "ACcodeIDE"
