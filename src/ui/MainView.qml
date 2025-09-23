import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GCS 1.0

ColumnLayout {
  id: main
  anchors.fill: parent
  spacing: 6

  // selected and active vehicle
  property var selectedVehicle: null
  property var v: (selectedVehicle ? selectedVehicle
                                   : (QMLGL.mvm.vehicles.length > 0 ? QMLGL.mvm.vehicles[0] : null))

  // local altitude target (not overwritten by telemetry)
  property real altInput: 10

  function modeName(vh) {
    if (!vh) return "-"
    const m = parseInt(vh.mode)
    switch (m) {
      case 0:  return "Stabilize"
      case 3:  return "Auto"
      case 4:  return "Guided"
      case 5:  return "Loiter"
      case 6:  return "RTL"
      case 7:  return "Circle"
      case 9:  return "Land"
      case 16: return "PosHold"
      case 17: return "Brake"
      default: return "" + vh.mode
    }
  }

  // TOP BAR
  Rectangle {
    Layout.fillWidth: true
    Layout.preferredHeight: 64
    color: "#2b2f34"
    border.color: "#1f2327"; border.width: 1

    RowLayout {
      anchors.fill: parent
      anchors.margins: 8
      spacing: 12

      Text { text: "Vehicle"; color: "white"; font.bold: true }

      ComboBox {
        id: vehicleBox
        Layout.preferredWidth: 160
        model: QMLGL.mvm.vehicles
        delegate: ItemDelegate {
          width: vehicleBox.width
          text: "SYSID " + (QMLGL.mvm.vehicles[index] ? QMLGL.mvm.vehicles[index].sysId : "?")
        }
        displayText: main.v ? ("SYSID " + main.v.sysId) : ""
        onActivated: (idx) => {
          main.selectedVehicle = QMLGL.mvm.vehicles[idx]
          main.altInput = main.v ? Math.max(5, main.v.position.altitude) : 10
        }
        Component.onCompleted: {
          if (QMLGL.mvm.vehicles.length > 0 && !main.selectedVehicle) {
            vehicleBox.currentIndex = 0
            main.selectedVehicle = QMLGL.mvm.vehicles[0]
            main.altInput = main.v ? Math.max(5, main.v.position.altitude) : 10
          }
        }
        Connections {
          target: QMLGL.mvm
          function onVehiclesChanged() {
            if (!main.selectedVehicle && QMLGL.mvm.vehicles.length > 0) {
              vehicleBox.currentIndex = 0
              main.selectedVehicle = QMLGL.mvm.vehicles[0]
              main.altInput = main.v ? Math.max(5, main.v.position.altitude) : 10
            }
            if (main.selectedVehicle && QMLGL.mvm.vehicles.indexOf(main.selectedVehicle) < 0) {
              main.selectedVehicle = QMLGL.mvm.vehicles.length ? QMLGL.mvm.vehicles[0] : null
              vehicleBox.currentIndex = main.selectedVehicle ? 0 : -1
              main.altInput = main.v ? Math.max(5, main.v.position.altitude) : 10
            }
          }
        }
      }

      Rectangle { width: 1; height: 28; color: "#444" }

      // UDP Port control (spec: must be configurable from UI)
      RowLayout {
        spacing: 6
        Text { text: "UDP"; color: "white" }
        SpinBox {
          id: portSpin
          from: 1024; to: 65535
          editable: true
          value: QMLGL.udpPort
        }
        Button {
          text: "Apply"
          onClicked: QMLGL.setUdpPort(portSpin.value)
        }
      }

      Rectangle { width: 1; height: 28; color: "#444" }

      // Telemetry line
      Flow {
        Layout.fillWidth: true
        spacing: 14
        Text { color: "white"; text: main.v ? ("System ID: " + main.v.sysId) : "System ID: -" }
        Text { color: "#bbb"; text: "•" }
        Text { color: "white"; text: main.v ? ("Status: " + main.v.status) : "Status: -" }
        Text { color: "#bbb"; text: "•" }
        Text { color: "white"; text: main.v ? ("Flight Mode: " + modeName(main.v)) : "Flight Mode: -" }
        Text { color: "#bbb"; text: "•" }
        Text { color: "white"; text: main.v ? ("Latitude: " + main.v.position.latitude.toFixed(6)) : "Latitude: -" }
        Text { color: "white"; text: main.v ? ("Longitude: " + main.v.position.longitude.toFixed(6)) : "Longitude: -" }
        Text { color: "#bbb"; text: "•" }
        Text { color: "white"; text: main.v ? ("Altitude (Rel): " + main.v.position.altitude.toFixed(1) + " m") : "Altitude (Rel): -" }
        Text { color: "white"; text: main.v ? ("Altitude (AMSL): " + main.v.altitudeAMSL.toFixed(1) + " m") : "Altitude (AMSL): -" }
        Text { color: "#bbb"; text: "•" }
        Text { color: "white"; text: main.v ? ("Ground Speed: " + main.v.groundSpeed.toFixed(1) + " m/s") : "Ground Speed: -" }
        Text { color: "white"; text: main.v ? ("Air Speed: " + main.v.airSpeed.toFixed(1) + " m/s") : "Air Speed: -" }
        Text { color: "#bbb"; text: "•" }
        Text { color: "white"; text: main.v ? ("Heading: " + main.v.heading.toFixed(1) + "°") : "Heading: -" }
        Text { color: "#bbb"; text: "•" }
        Text { color: "white"; text: main.v ? ("Battery: " + main.v.batteryPct + "%") : "Battery: -" }
      }
    }
  }

  // MAP
  MapView {
    Layout.fillWidth: true
    Layout.fillHeight: true
    selectedVehicle: main.v
  }

  // COMMANDS (unchanged logic + Brake)
  Rectangle {
    id: cmdBar
    Layout.fillWidth: true
    implicitHeight: cmdCol.implicitHeight + 16
    color: "#f4f5f6"; border.color: "#c8c8c8"

    ColumnLayout {
      id: cmdCol
      anchors.fill: parent
      anchors.margins: 10
      spacing: 8

      Text { text: "Commands"; font.bold: true; color: "black" }

      RowLayout {
        spacing: 8
        Button { text: "Arm";     visible: main.v && !main.v.armed;                   enabled: !!main.v; onClicked: main.v.arm(true) }
        Button { text: "Disarm";  visible: main.v &&  main.v.armed && !main.v.inAir;  enabled: !!main.v; onClicked: main.v.arm(false) }
        Button { text: "Takeoff"; visible: main.v &&  main.v.armed && !main.v.inAir;  enabled: !!main.v; onClicked: main.v.takeoff(main.altInput) }
        Button { text: "Land";    visible: main.v &&  main.v.inAir;                   enabled: !!main.v; onClicked: main.v.land() }
        Button { text: "RTL";     visible: main.v &&  main.v.armed;                   enabled: !!main.v; onClicked: main.v.rtl() }
        Button { text: "Brake";   visible: main.v &&  main.v.inAir;                   enabled: !!main.v; onClicked: main.v.setModeText("Brake") }
      }

      RowLayout {
        spacing: 8
        visible: !!main.v
        Text { text: "Altitude (m)"; color: "black" }
        SpinBox {
          id: altSpin
          from: 1; to: 1000
          editable: true
          value: main.altInput
          onValueModified: main.altInput = value
        }
        Button {
          text: "Change Altitude"
          enabled: !!main.v
          onClicked: main.v.changeAltitude(main.altInput)
        }
      }

      RowLayout {
        spacing: 8
        Text { text: "Flight Mode"; color: "black" }
        ComboBox {
          id: modeBox
          model: ["Guided","Position Hold","Brake","Land","RTL"]
          enabled: !!main.v
          onActivated: main.v.setModeText(currentText === "Position Hold" ? "PosHold" : currentText)
        }
      }
    }
  }
}
