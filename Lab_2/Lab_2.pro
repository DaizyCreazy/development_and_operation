QT += quick
QT += widgets
QT += printsupport
QT += gui

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    backend.cpp \
    main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Data.json \

HEADERS += \
    backend.h

include (D:\Notebook\Labs_Kavallini_181_331\Lab_2\SortFilterProxyModel-master/SortFilterProxyModel.pri)


INCLUDEPATH += D:\Notebook\Qt\Tools\OpenSSL\Win_x64\include
LIBS += -L"D:\Notebook\Qt\Tools\OpenSSL\Win_x64\bin"
LIBS += -L"D:\Notebook\Qt\Tools\OpenSSL\Win_x64\lib" -llibssl
LIBS += -L"D:\Notebook\Qt\Tools\OpenSSL\Win_x64\lib" -llibcrypto



