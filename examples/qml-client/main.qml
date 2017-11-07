import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import TelegramQt 1.0
import TelegramQtTheme 1.0

Window {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("TelegramQt Example")

    AppInformation {
        id: appInfo
        appId: 14617
        appHash: "e17ac360fd072f83d5d08db45ce9a121" // Telepathy-Morse app hash
        appVersion: "0.1"
        deviceInfo: "pc"
        osInfo: "GNU/Linux"
        languageCode: "en"
    }

    TelegramCore {
        id: telegramCore
        updatesEnabled: false
        applicationInformation: appInfo
    }

    Flickable {
        id: view
        anchors.fill: parent
        anchors.margins: 20
        Column {
            id: mainColumn
            width: window.width - view.anchors.margins * 2
            TelegramAddAccount {
                id: telegramAddAccount
                width: parent.width
            }

            ListModel {
                id: debugDataModel
                function addMessage(message)
                {
                    console.log(message)
                    append({"timestamp": Qt.formatDateTime(new Date(), "hh:mm:ss"), "message": message })
                }
            }

            Switch {
    //        TextSwitch {
                id: debugViewSwitch
                text: qsTr("Show debug data")
    //            description: "Check this to view logs"
            }

            Repeater {
                id: logRepeater
                model: debugViewSwitch.checked ? debugDataModel : 0
                Row {
                    spacing: Theme.paddingSmall
                    Label {
                        text: model.timestamp
                    }
                    Label {
                        text: model.message
                        wrapMode: Text.Wrap
                        width: mainColumn.width - x
                    }
                }
            }
            Label {
                visible: debugViewSwitch.checked && (logRepeater.count === 0)
                text: qsTr("There is no log messages yet")
            }
        }
    }

    Shortcut {
        sequence: StandardKey.Quit
        onActivated: window.close()
    }
}
