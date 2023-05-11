import QtQuick 2.0
import opacityimage 1.0
import QtQuick.Shapes 1.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Column{
        spacing: 10
        OpacityImage {
            width: 300
            height: 300
            source: "image1"
            radius: 20
            resizemode: OpacityImage.Fixed
            border: 0
            borderColor: "red"
            gradient: {
                "type": "linear",
                "xStart": 0,
                "yStart": 0,
                "xStop": width,
                "yStop": height,
                "stops": [
                            { "position": 0, "color": "blue" },
                            { "position": 0.5, "color": "black" },
                            { "position": 1, "color": "white" }
                        ]
            }
        }

        Item {
            width: parent.width
            height: 80
            clip: true
            OpacityImage {
                width: 300
                height: 300
                source: "image1"
                radius: 20
                resizemode: OpacityImage.Fixed
                xMirror: true
                gradient: {
                    "type": "linear",
                    "xStart": 0,
                    "yStart": 0,
                    "xStop": width,
                    "yStop": height,
                    "stops": [
                                { "position": 0, "color": "blue" },
                                { "position": 0.5, "color": "black" },
                                { "position": 1, "color": "white" }
                            ]
                }
            }

        }

    }
}
