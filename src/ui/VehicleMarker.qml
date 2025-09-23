import QtQuick
import QtLocation
import QtPositioning

MapItemGroup {
  id: root
  required property var vehicle

  // Drone triangle
  MapQuickItem {
    id: drone
    z: 1000
    visible: !!root.vehicle && root.vehicle.position && root.vehicle.position.isValid
    anchorPoint.x: 12
    anchorPoint.y: 12
    coordinate: QtPositioning.coordinate(root.vehicle.position.latitude,
                                         root.vehicle.position.longitude)
    rotation: root.vehicle.heading
    sourceItem: Canvas {
      width: 24; height: 24
      onPaint: {
        const ctx = getContext("2d");
        ctx.reset();
        ctx.beginPath();
        ctx.moveTo(width/2, 2);
        ctx.lineTo(width-2, height-2);
        ctx.lineTo(2, height-2);
        ctx.closePath();
        ctx.fillStyle = "#ffcc66";
        ctx.fill();
        ctx.lineWidth = 1;
        ctx.strokeStyle = "#222";
        ctx.stroke();
        ctx.beginPath();
        ctx.arc(width/2, height/2, 2.2, 0, Math.PI*2);
        ctx.fillStyle = "#222";
        ctx.fill();
      }
    }
  }

  // Home marker
  MapQuickItem {
    z: 900
    visible: root.vehicle && root.vehicle.home && root.vehicle.home.isValid
    anchorPoint.x: 6; anchorPoint.y: 6
    coordinate: QtPositioning.coordinate(root.vehicle.home.latitude, root.vehicle.home.longitude)
    sourceItem: Rectangle { width: 12; height: 12; color: "green"; radius: 2; border.color: "black" }
  }

  // Go-to marker
  MapQuickItem {
    z: 900
    visible: root.vehicle && root.vehicle.gotoPoint && root.vehicle.gotoPoint.isValid
    anchorPoint.x: 6; anchorPoint.y: 6
    coordinate: QtPositioning.coordinate(root.vehicle.gotoPoint.latitude, root.vehicle.gotoPoint.longitude)
    sourceItem: Rectangle { width: 12; height: 12; color: "orange"; radius: 2; border.color: "black" }
  }
}
