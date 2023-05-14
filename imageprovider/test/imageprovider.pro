QT += quick core gui

SOURCES += \
        ../imageprovider.cpp \
        main.cpp

HEADERS += ../imageprovider.h \
        CallManager.h

resources.files = main.qml
resources.prefix = /$${TARGET}
RESOURCES += resources \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    image1.jpg \
    image2.jpg \
    image3.jpg \
    image4.jpg \
    image5.png
