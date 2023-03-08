import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15

Item {
    id: root

    property string imageSource: ""
    property int radius: 0
    property variant gradient: Gradient { }
    property variant gradientStartPoint: Qt.point(root.x + root.width / 2, root.y)
    property variant gradientStopPoint: Qt.point(root.x + root.width /2, root.y + root.height)

    Image {
        id: image
        source: imageSource
        visible: false
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d");
            var rectWidth = width;
            var rectHeight = height;
            var cornerRadius = radius;
            var x = 0;
            var y = 0;

            ctx.save();
            ctx.clearRect(0, 0, rectWidth, rectHeight);

            // Create a linear gradient
            var x0 = root.gradientStartPoint.x;
            var y0 = root.gradientStartPoint.y;
            var x1 = root.gradientStopPoint.x;
            var y1 = root.gradientStopPoint.y;

            var gradient = ctx.createLinearGradient(x0, y0, x1, y1);
            var gradientEnable = root.gradient.stops.length > 0? true: false;

            for(var i = 0; i < root.gradient.stops.length; i++){
                var position = root.gradient.stops[i].position;
                var color = root.gradient.stops[i].color;
                gradient.addColorStop(position, color);
            }

            /* Create rounded border */
            ctx.beginPath();
            ctx.moveTo(x + cornerRadius, y);
            ctx.lineTo(x + rectWidth - cornerRadius, y);
            ctx.arcTo(x + rectWidth, y, x + rectWidth, y + cornerRadius, cornerRadius);
            ctx.lineTo(x + rectWidth, y + rectHeight - cornerRadius);
            ctx.arcTo(x + rectWidth, y + rectHeight, x + rectWidth - cornerRadius, y + rectHeight, cornerRadius);
            ctx.lineTo(x + cornerRadius, y + rectHeight);
            ctx.arcTo(x, y + rectHeight, x, y + rectHeight - cornerRadius, cornerRadius);
            ctx.lineTo(x, y + cornerRadius);
            ctx.arcTo(x, y, x + cornerRadius, y, cornerRadius);
            ctx.closePath();

            // restrict the drawing area to the rounded rectangle path
            ctx.clip();

            if(image.status === Image.Ready){
                // draw the image inside the rounded rectangle, scaling it to fit
                ctx.drawImage(image, x, y, rectWidth, rectHeight);
            }

            // Fill the rectangle with the gradient
            if(gradientEnable){
                ctx.fillStyle = gradient;
                ctx.fill();
            }
            ctx.restore();
        }
    }
}
