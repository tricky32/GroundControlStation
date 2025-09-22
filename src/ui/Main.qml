import QtQuick
import QtQuick.Window
import gcs_application

Window {
    id: root
    width: 1280
    height: 800
    visible: true
    title: "gcs_application"

    MainView {
        anchors.fill: parent
    }
}
