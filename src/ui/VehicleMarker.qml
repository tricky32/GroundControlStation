import QtQuick 2.15

Item {
    id: root
    // incoming data object (C++ Vehicle as QObject)
    property var v

    width: 28; height: 28
    visible: v && v.position && v.position.isValid

    // rotate the glyph by heading
    rotation: v && v.heading ? v.heading : 0
    antialiasing: true

    Canvas {
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            // triangle pointer
            ctx.beginPath()
            ctx.moveTo(width/2, 2)
            ctx.lineTo(width-4, height-4)
            ctx.lineTo(4, height-4)
            ctx.closePath()
            ctx.lineWidth = 2
            ctx.strokeStyle = "#222"
            ctx.stroke()
            ctx.fillStyle = (root.v && root.v.armed) ? "#18a558" : "#d9534f"
            ctx.fill()
        }
    }
}
