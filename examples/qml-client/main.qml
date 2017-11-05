import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.0

Window {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("TelegramQt Example")

    Flickable {
        id: view
        anchors.fill: parent
        anchors.margins: 20
        Column {
            width: window.width - view.anchors.margins * 2
            TelegramAddAccount {
                width: parent.width
            }
        }
    }

    Shortcut {
        sequence: StandardKey.Quit
        onActivated: window.close()
    }
}
