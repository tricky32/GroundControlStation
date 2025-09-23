import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GCS 1.0

ColumnLayout {
  id: main
  anchors.fill: parent
  spacing: 6

  // selected vehicle (or null)
  property var selectedVehicle: null
  // active vehicle: selected or first available
  property var v: (selectedVehicle ? selectedVehicle
                                   : (QMLGL.mvm.vehicles.length > 0 ? QMLGL.mvm.vehicles[0] : null))

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

  // ── TOP BAR: selector + telemetry
  Rectangle {
    Layout.fillWidth: true
    Layout.preferredHeight: 56
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
        onActivated: (idx) => main.selectedVehicle = QMLGL.mvm.vehicles[idx]

        Component.onCompleted: {
          if (QMLGL.mvm.vehicles.length > 0 && !main.selectedVehicle) {
            vehicleBox.currentIndex = 0
            main.selectedVehicle = QMLGL.mvm.vehicles[0]
          }
        }
        Connections {
          target: QMLGL.mvm
          function onVehiclesChanged() {
            if (!main.selectedVehicle && QMLGL.mvm.vehicles.length > 0) {
              vehicleBox.currentIndex = 0
              main.selectedVehicle = QMLGL.mvm.vehicles[0]
            }
            if (main.selectedVehicle && QMLGL.mvm.vehicles.indexOf(main.selectedVehicle) < 0) {
              main.selectedVehicle = QMLGL.mvm.vehicles.length ? QMLGL.mvm.vehicles[0] : null
              vehicleBox.currentIndex = main.selectedVehicle ? 0 : -1
            }
          }
        }
      }

      Rectangle { width: 1; height: 28; color: "#444" }

      // Telemetry line (names + "-" fallbacks)
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

  // ── MAP
  MapView {
    Layout.fillWidth: true
    Layout.fillHeight: true
    selectedVehicle: main.v
  }

  // ── COMMANDS (stable visibilities; solid background)
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
        // Drive visibility from booleans provided by Vehicle (no blinking)
        Button { text: "Arm";     visible: main.v && !main.v.armed;                 enabled: !!main.v; onClicked: main.v.arm(true) }
        Button { text: "Disarm";  visible: main.v &&  main.v.armed && !main.v.inAir; enabled: !!main.v; onClicked: main.v.arm(false) }
        Button { text: "Takeoff"; visible: main.v &&  main.v.armed && !main.v.inAir; enabled: !!main.v; onClicked: main.v.takeoff(altSpin.value) }
        Button { text: "Land";    visible: main.v &&  main.v.inAir;                  enabled: !!main.v; onClicked: main.v.land() }
        Button { text: "RTL";     visible: main.v &&  main.v.armed;                  enabled: !!main.v; onClicked: main.v.rtl() }
      }

      RowLayout {
        spacing: 8
        visible: main.v && main.v.armed
        Text { text: "Altitude (m)"; color: "black" }
        SpinBox {
          id: altSpin; from: 1; to: 500; editable: true
          value: (main.v ? Math.max(5, main.v.position.altitude) : 10)
        }
        Button { text: "Change Altitude"; enabled: !!main.v; onClicked: main.v.changeAltitude(altSpin.value) }
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
