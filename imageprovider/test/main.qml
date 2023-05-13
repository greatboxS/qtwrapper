import QtQuick 2.0
import opacityimage 1.0
import QtQuick.Shapes 1.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    property var images: ["image1.jpg", "image2.jpg", "image3.jpg", "image4.jpg"]
    property int imageIndex: 0
    property string  imageName: images[imageIndex]

    Timer {
        interval: 2000
        repeat: true
        running: true
        onTriggered: {
            imageIndex ++;
            if(imageIndex >=4) imageIndex = 0;
            imageName = images[imageIndex]
        }
    }

    Rectangle {
        // color: "lightblue"
        anchors.fill: parent;

        Text {
            text: "dfsdfsefsefs"
        }

        Column{
            spacing: 10
            OpacityImage {
                id: img1
                width: 300
                height: 300
                source: imageName
                radius: 10
                resizemode: OpacityImage.Fit
            //   gradient: {
            //         "type": "linear",
            //         "Cx": 50.0,
            //         "Cy": 50.0,
            //         "Fx": 50.0,
            //         "Fy": 50.0,
            //         "fradius": 50.0,
            //         "cradius": 50.0,
            //         "angle": 100.0,
            //         "radius": 100,
            //         "stops": [
            //                   { "position": 1.0, "color": "#ff000000" },
            //                   { "position": 0.5, "color": "#10282828" },
            //                   { "position": 0.0, "color": "#0f282828" }
            //               ]
            //   }
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
                    gradient: {
                        "type": "linear",
                        "stops": [
                                    { "position": 0.0, "color": "#ff000000" },
                                    { "position": 0.2, "color": "#00000000" },
                                ]
                    }
                }

            }

        }
    }
}
