import QtQuick 2.0
import opacityimage 1.0
import QtQuick.Shapes 1.15
import QtQuick.Controls 2.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    property var images: [
        "qrc:/image/image1.jpg",
        "qrc:/image/image2.jpg",
        "qrc:/image/image3.jpg",
        "qrc:/image/image4.jpg",
        "qrc:/image/image5.png"
    ]
    property int imageIndex: 0
    property string  imageName: images[imageIndex]
    property double sliderVal: 1

    Timer {
        interval: 2000
        repeat: true
        running: true
        onTriggered: {
            imageIndex ++;
            if(imageIndex >=5) imageIndex = 0;
            imageName = images[imageIndex]
        }
    }

    Rectangle {
        // color: "lightblue"
        anchors.fill: parent;
        Slider {
            id: slider
            anchors.top: imagePannel.bottom
            value: sliderVal
            onValueChanged: {
                console.log(sliderVal);
                sliderVal = value;
            }
        }

        Row {
            id: imagePannel
            spacing: 3
            Repeater {
                anchors.fill: parent
                model: 10
                delegate: Rectangle {
                    Text {
                        text: "dfsdfsefsefs"
                    }

                    width: 300
                    height: 500
                    Column{
                        spacing: 10
                        anchors.fill: parent
                        OpacityImage {
                            id: img1
                            width: 300
                            height: 300
                            url: imageName
                            radius: 10
                            opacity: sliderVal
                            resizemode: OpacityImage.Fit
                        }

                        Item {
                            width: parent.width
                            height: 100
                            clip: true
                            OpacityImage {
                                id: img2
                                width: 300
                                height: 300
                                source: imageName
                                radius: 10
                                resizemode: OpacityImage.Fit
                                xMirror: true
                                gradient: LinearGradient
                                {
                                    x1: 0
                                    y1: 0
                                    x2: 0
                                    y2: height
                                    GradientStop { position: 0.0; color: "#ff000000"}
                                    GradientStop { position: 0.1; color: "#0f111111"}
                                    GradientStop { position: 0.3; color: "#03111111"}
                                    GradientStop { position: 1.0; color: "#00000000"}
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
