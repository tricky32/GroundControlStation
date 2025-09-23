import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning
import GCS 1.0

Item {
  id: root
  // parent layout sizes this
  property var selectedVehicle: null

  // goto popup state
  property var pendingCoord: undefined
  property real popupX: 0
  property real popupY: 0

  // SVG path inside the QML module (adjust if you changed the URI)
  readonly property string resPrefix: "qrc:/qt/qml/gcs_application/res/"

  // one-time auto-centering flag
  property bool autoCentered: false

  Plugin {
    id: osm
    name: "osm"
    PluginParameter { name: "osm.mapping.highdpi_tiles"; value: true }
    PluginParameter { name: "osm.mapping.host"; value: "https://tile.openstreetmap.org" }
  }

  Map {
    id: map
    anchors.fill: parent
    plugin: osm
    zoomLevel: 16

    // ---- auto-center ONCE when first valid position arrives ----
    Timer {
      interval: 250
      running: !root.autoCentered
      repeat: true
      onTriggered: {
        const v = root.selectedVehicle || (QMLGL.mvm.vehicles.length ? QMLGL.mvm.vehicles[0] : null)
        if (v && v.position && v.position.isValid) {
          map.center = QtPositioning.coordinate(v.position.latitude, v.position.longitude)
          root.autoCentered = true
          running = false
        }
      }
    }

    // --- DRONE + HOME + GOTO markers (SVG icons) ---
    MapItemView {
      model: QMLGL.mvm.vehicles.length
      delegate: MapItemGroup {
        property var v: QMLGL.mvm.vehicles[index]

        // Drone icon (rotates with heading)
        MapQuickItem {
          id: droneItem
          z: 1000
          visible: !!v && v.position && v.position.isValid
          anchorPoint.x: droneImg.width * 0.5
          anchorPoint.y: droneImg.height * 0.5
          coordinate: visible ? QtPositioning.coordinate(v.position.latitude, v.position.longitude)
                               : QtPositioning.coordinate()
          rotation: visible ? v.heading : 0
          sourceItem: Image {
            id: droneImg
            source: resPrefix + "example.svg"
            sourceSize.width: 32   // tweak sizes here if you want bigger/smaller
            sourceSize.height: 32
            smooth: true
            antialiasing: true
          }
        }

        // Home icon (from HOME_POSITION)
        MapQuickItem {
          z: 900
          visible: !!v && v.home && v.home.isValid
          anchorPoint.x: homeImg.width * 0.5
          anchorPoint.y: homeImg.height * 0.5
          coordinate: visible ? QtPositioning.coordinate(v.home.latitude, v.home.longitude)
                               : QtPositioning.coordinate()
          sourceItem: Image {
            id: homeImg
            source: resPrefix + "home.svg"
            sourceSize.width: 22
            sourceSize.height: 22
            smooth: true
            antialiasing: true
          }
        }

        // GoTo icon (hide automatically in Brake/RTL)
        MapQuickItem {
          z: 900
          readonly property bool modeBlocks: !!v && (parseInt(v.mode) === 17 || parseInt(v.mode) === 6)
          visible: !!v && v.gotoPoint && v.gotoPoint.isValid && !modeBlocks
          anchorPoint.x: gotoImg.width * 0.5
          anchorPoint.y: gotoImg.height * 0.5
          coordinate: visible ? QtPositioning.coordinate(v.gotoPoint.latitude, v.gotoPoint.longitude)
                               : QtPositioning.coordinate()
          sourceItem: Image {
            id: gotoImg
            source: resPrefix + "goto.svg"
            sourceSize.width: 20
            sourceSize.height: 20
            smooth: true
            antialiasing: true
          }
        }
      }
    }

    // Pan with left-drag
    property var _originCenter: null
    DragHandler {
      target: null
      onActiveChanged: map._originCenter = active ? map.center : null
      onTranslationChanged: {
        if (!map._originCenter) return
        const px = map.fromCoordinate(map._originCenter)
        map.center = map.toCoordinate(Qt.point(px.x - translation.x, px.y - translation.y))
      }
    }

    // Wheel zoom
    WheelHandler {
      target: null
      onWheel: (w) => {
        const steps = w.angleDelta.y / 120
        map.zoomLevel = Math.max(2, Math.min(20, map.zoomLevel + steps * 0.4))
        w.accepted = true
      }
    }

    // Right-click → goto popup
    MouseArea {
      anchors.fill: parent
      acceptedButtons: Qt.RightButton
      propagateComposedEvents: true
      onPressed: (mouse) => {
        if (mouse.button === Qt.RightButton) {
          root.pendingCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
          root.popupX = mouse.x
          root.popupY = mouse.y
          gotoPopup.open()
          mouse.accepted = true
        }
      }
    }

    // Center button (manual recentre)
    Button {
      text: "Center"
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.margins: 8
      onClicked: {
        const v = root.selectedVehicle || (QMLGL.mvm.vehicles.length ? QMLGL.mvm.vehicles[0] : null)
        if (v) map.center = QtPositioning.coordinate(v.position.latitude, v.position.longitude)
      }
    }

    // Altitude bar
    Rectangle {
      id: altBar
      width: 10
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: 12
      readonly property var v: root.selectedVehicle || (QMLGL.mvm.vehicles.length ? QMLGL.mvm.vehicles[0] : null)
      height: Math.min(200, (v ? v.position.altitude : 0) * 3)
      color: "#66bbff"; border.color: "#004"; border.width: 1
      HoverHandler { id: hh }
      ToolTip.visible: hh.hovered
      ToolTip.text: (v ? (v.position.altitude.toFixed(1) + " m") : "")
    }
  }

  // goto popup
  Popup {
    id: gotoPopup
    x: root.popupX
    y: root.popupY
    padding: 8
    modal: false
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    contentItem: Column {
      spacing: 6
      Text {
        text: root.pendingCoord
              ? ("send goto coords: " +
                 root.pendingCoord.latitude.toFixed(6) + ", " +
                 root.pendingCoord.longitude.toFixed(6))
              : "send goto coords:"
      }
      Row {
        spacing: 8
        Button {
          text: "Confirm"
          enabled: root.pendingCoord !== undefined && (root.selectedVehicle || QMLGL.mvm.vehicles.length > 0)
          onClicked: {
            const v = root.selectedVehicle || (QMLGL.mvm.vehicles.length ? QMLGL.mvm.vehicles[0] : null)
            if (v && root.pendingCoord) {
              const alt = Math.max(5, v.position.altitude)
              v.gotoLatLonAlt(root.pendingCoord.latitude, root.pendingCoord.longitude, alt)
            }
            gotoPopup.close()
          }
        }
        Button { text: "Cancel"; onClicked: gotoPopup.close() }
      }
    }
  }
}
