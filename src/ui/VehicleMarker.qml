import QtQuick
import QtLocation
import QtPositioning

MapItemGroup {
  id: root
  required property var vehicle

  // Kept for compatibility (unused by current MapView)
  visible: false
}
