import QtQuick

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Image {
        width: 64
        height: 64
        source: "image://imageProvider/image1"
    }
}
