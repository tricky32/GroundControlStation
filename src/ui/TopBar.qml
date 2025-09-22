import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property int   activeSysId: -1
    property var   activeVehicle: null
    signal centerRequested()

    height: 44
    color: "#15171a"; border.color: "#262a2f"

    Row {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 12

        Label { text: "GCS"; color: "#e6e6e6"; font.bold: true }
        Label {
            text: "Vehicles: " + (MVM && MVM.vehicles ? MVM.vehicles.length : 0)
            color: "#cfd3da"
        }
        Item { width: 12; height: 1 }
        Button {
            text: "Center on Active"
            onClicked: root.centerRequested()
        }
        Item { width: 12; height: 1 }
        Label {
            text: activeVehicle
                  ? ("Mode: " + (activeVehicle.mode ?? "—"))
                  : "Mode: —"
            color: "#cfd3da"
        }
    }
}
