import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property int sysId: -1
    property var vehicle: null

    signal arm()
    signal disarm()
    signal takeoff(real alt)
    signal land()
    signal rtl()
    signal modeSelected(string modeName)
    signal changeAltitude(real alt)

    height: 56
    color: "#15171a"; border.color: "#262a2f"

    Row {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Arm/Disarm
        Button {
            text: "Arm"
            visible: vehicle && vehicle.armed === false
            onClicked: root.arm()
        }
        Button {
            text: "Disarm"
            visible: vehicle && vehicle.armed === true
            onClicked: root.disarm()
        }

        // Takeoff
        Row {
            spacing: 6
            visible: vehicle && vehicle.armed === true
            Button { text: "Takeoff"; onClicked: root.takeoff(tAlt.value) }
            SpinBox { id: tAlt; from: 1; to: 100; value: 10; stepSize: 1 }
            Label { text: "m"; color: "#cfd3da" }
        }

        // Land / RTL
        Button {
            text: "Land"
            visible: vehicle && vehicle.armed === true
            onClicked: root.land()
        }
        Button {
            text: "RTL"
            visible: vehicle && vehicle.armed === true
            onClicked: root.rtl()
        }

        // Change Alt
        Row {
            spacing: 6
            visible: vehicle && vehicle.armed === true
            Button { text: "Change Alt"; onClicked: root.changeAltitude(chAlt.value) }
            SpinBox { id: chAlt; from: 1; to: 200; value: 15; stepSize: 1 }
            Label { text: "m"; color: "#cfd3da" }
        }

        // Mode selector
        Item { width: 12; height: 1 }
        Label { text: "Mode:"; color: "#cfd3da"; verticalAlignment: Text.AlignVCenter }
        ComboBox {
            id: modeBox
            width: 180
            textRole: "name"
            model: [
                { name: "Guided",         custom: 4  },
                { name: "Position Hold",  custom: 16 },
                { name: "Brake",          custom: 17 },
                { name: "Land",           custom: 9  },
                { name: "RTL",            custom: 6  }
            ]
            onActivated: {
                const sel = model[index]
                root.modeSelected(sel.name)
            }
        }

        // Telemetry summary
        Item { width: 18; height: 1 }
        Label {
            text: vehicle
                  ? `Alt ${vehicle.alt !== undefined ? vehicle.alt.toFixed(1) : "—"} m`
                  : "Alt —"
            color: "#cfd3da"
        }
        Label {
            text: vehicle
                  ? `GS ${vehicle.groundspeed !== undefined ? vehicle.groundspeed.toFixed(1) : "—"} m/s`
                  : "GS —"
            color: "#cfd3da"
        }
        Label {
            text: vehicle
                  ? `AS ${vehicle.airspeed !== undefined ? vehicle.airspeed.toFixed(1) : "—"} m/s`
                  : "AS —"
            color: "#cfd3da"
        }
    }
}
