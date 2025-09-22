import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 1200; height: 800

    // Top bar
    Rectangle {
        id: topbar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 40
        color: "#1e1e1e"
        opacity: 0.95
        z: 10

        Text {
            text: "GCS — UDP 5760"
            color: "white"
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 12
        }
    }

    // Map
    MapView {
        id: mapView
        anchors.top: topbar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
