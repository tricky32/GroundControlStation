import QtQuick 2.15
import QtLocation 6.5
import QtPositioning 6.5

Item {
    id: root
    anchors.fill: parent

    // OSM plugin (works without keys; you just need network access)
    Plugin { id: osm; name: "osm" }

    Map {
        id: map
        anchors.fill: parent
        plugin: osm
        zoomLevel: 15
        // Tallinn default so you see *something* even before a vehicle appears
        center: QtPositioning.coordinate(59.4370, 24.7536)

        // Basic handlers to pan/zoom without MapGestureArea
        DragHandler { target: null; acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onActiveChanged: if (active) map.pan(-translation.x, -translation.y)
            onTranslationChanged: map.pan(-translation.x, -translation.y)
        }
        WheelHandler { target: null
            onWheel: (event) => {
                // zoom towards cursor
                const dir = event.angleDelta.y > 0 ? 1 : -1
                map.zoomLevel = Math.max(2, Math.min(20, map.zoomLevel + dir))
            }
        }

        // Vehicle markers (blue dots)
        Repeater {
            model: mvm ? mvm.vehicles : []
            delegate: MapQuickItem {
                id: marker
                parent: map
                coordinate: modelData.position
                anchorPoint.x: 6
                anchorPoint.y: 6
                sourceItem: Rectangle {
                    width: 12; height: 12; radius: 6
                    border.width: 1; border.color: "white"
                    color: "#2d6cdf"
                    ToolTip.visible: ma.containsMouse
                    ToolTip.text: "SYSID " + modelData.sysId +
                                  "\nMode: " + modelData.mode +
                                  "\nStatus: " + modelData.status
                    MouseArea { id: ma; anchors.fill: parent; hoverEnabled: true }
                }
            }
        }
    }
}
