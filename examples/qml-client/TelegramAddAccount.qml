import QtQuick 2.2
import QtQuick.Controls 2.0
import Qt.labs.platform 1.0
import TelegramQt 1.0
import TelegramQtTheme 1.0

Column {
    width: 800
    id: telegramCommonColumn
    property bool editMode: true
    property alias phoneNumber: phoneNumberField.text
    property bool acceptableInput: secretHelper.credentialDataExists
    property bool showDetails: telegramCore.connected || secretHelper.credentialDataExists
    property int phoneRegistrationStatus: registrationStatus.unknown
    property int authError: 0

    QtObject {
        id: registrationStatus
        readonly property int unknown: 0
        readonly property int notRegistered: 1
        readonly property int registered: 2
        readonly property int error: 3
    }

    AccountSecretHelper {
        id: secretHelper
        secretDirectory: StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/.cache/telepathy-morse/secrets"
        phoneNumber: phoneNumberField.text
        format: AccountSecretHelper.FormatBinary
    }

    QtObject {
        id: authHelper

        function checkPhone(phone)
        {
            pendingPhoneNumberForCheck = phone
            if (!connected) {
                debugDataModel.addMessage("requestPhoneStatus: " + phone + " (pending)")
                tryToConnect()
                return
            }

            debugDataModel.addMessage("requestPhoneStatus: " + phone)
            requestPhoneStatus(phone)
        }

        property int connectionState: telegramCore.connectionState
        property bool connected: connectionState >= TelegramNamespace.ConnectionStateConnected
        property bool phoneNumberNeeded: needsState === needsPhoneNumber
        property bool authCodeNeeded: needsState === needsAuthCode
        property bool passwordNeeded: needsState === needsPassword
        property bool busy: pendingPhoneNumberForCheck

        property int needsState: needsPhoneNumber
        readonly property int needsNothing: 0
        readonly property int needsPhoneNumber: 1
        readonly property int needsAuthCode: 2
        readonly property int needsPassword: 3
        readonly property bool hasValidCredentials: connectionState >= TelegramNamespace.ConnectionStateAuthenticated

        property string pendingPhoneNumberForCheck

        property string currentPhone: telegramCommonColumn.phoneNumber

        onCurrentPhoneChanged: telegramCommonColumn.phoneRegistrationStatus = registrationStatus.unknown

        property bool currentPhoneRegistered: false

        onConnectedChanged: {
            if (connected) {
                if (pendingPhoneNumberForCheck) {
                    debugDataModel.addMessage("Connected. Request status of " + pendingPhoneNumberForCheck)
                    telegramCore.requestPhoneStatus(pendingPhoneNumberForCheck)
                }
            }
        }

        function tryToConnect() {
            if (connectionState !== TelegramNamespace.ConnectionStateDisconnected) {
                debugDataModel.addMessage("Asked to connect, but the state is " + connectionState + " already")
                return
            }

            debugDataModel.addMessage("Init connection...")
            telegramCore.initConnection()
        }

        function requestAuthCode(phone) {
            debugDataModel.addMessage("Request auth code for phone number " + phone)
            telegramCore.requestPhoneCode(phone)
        }

        function trySignIn(phone, code) {
            debugDataModel.addMessage("Sign in account " + phone + " with code " + code)
            needsState = needsNothing
            signIn(phone, code)
        }

        function tryPassword2(password) {
            debugDataModel.addMessage("Try pass")
            needsState = needsNothing
            tryPassword(password)
        }

        function setReceivedPhoneStatus(phone, registered) {
            debugDataModel.addMessage("phoneStatusReceived: " + phone + " registered: " + registered)

            if (phone !== pendingPhoneNumberForCheck) {
                debugDataModel.addMessage("Got statuses of different phone, than requested: " + phone + " vs " + pendingPhoneNumberForCheck)
            }
            pendingPhoneNumberForCheck = ""
            if (phone === currentPhone) {
                if (registered) {
                    telegramCommonColumn.phoneRegistrationStatus = registrationStatus.registered
                } else {
                    telegramCommonColumn.phoneRegistrationStatus = registrationStatus.notRegistered
                }
            }
        }
    }

    Connections {
        target: telegramCore

        onPhoneStatusReceived: authHelper.setReceivedPhoneStatus(phone, registered)

        onPhoneCodeRequired: {
            debugDataModel.addMessage("Phone code required")
            needsState = needsAuthCode
        }

        onAuthSignErrorReceived: {
            debugDataModel.addMessage("AuthSignErrorReceived: " + errorMessage + " (code: " + errorCode + ")")
            switch (errorCode) {
            case TelegramNamespace.AuthSignErrorPhoneCodeIsInvalid:
                debugDataModel.addMessage("Phone code is not valid")
                needsState = needsAuthCode
                break
            case TelegramNamespace.AuthSignErrorAppIdIsInvalid:
            case TelegramNamespace.AuthSignErrorPhoneNumberIsInvalid:
            case TelegramNamespace.AuthSignErrorPhoneNumberIsOccupied:
            case TelegramNamespace.AuthSignErrorPhoneNumberIsUnoccupied:
                authError = errorCode
                telegramCommonColumn.phoneRegistrationStatus = registrationStatus.error;
                pendingPhoneNumberForCheck = ""
                break
            default:
                break;
            }
        }

        onAuthorizationErrorReceived: {
            debugDataModel.addMessage("AuthorizationErrorReceived: " + errorMessage + " (code: " + errorCode + ")")
            if (errorCode === TelegramNamespace.UnauthorizedSessionPasswordNeeded) {
                needsState = needsPassword

                passwordField.focus = true
                telegramCore.getPassword()
            }
        }

        onPasswordInfoReceived: {
            debugDataModel.addMessage("onPasswordInfoReceived")
        }

        onConnectionStateChanged: {
            debugDataModel.addMessage("Connection state changed to " + state)

//            if (state === TelegramNamespace.ConnectionStateAuthRequired) {
//                debugDataModel.addMessage("requestPhoneCode " + phoneNumberField.text)
//                authHelper.requestPhoneCode(phoneNumberField.text)
//                needsState = needsAuthCode
//            }

            if (state === TelegramNamespace.ConnectionStateAuthenticated) {
                debugDataModel.addMessage("Authenticated and ready!")
                needsState = needsNothing
            }
        }
        onLoggedOut: {
            debugDataModel.addMessage("Log out result: " + result)
        }
    }

    TextField {
        id: phoneNumberField
        readOnly: !editMode
        width: parent.width
        enabled: authHelper.phoneNumberNeeded && (authHelper.pendingPhoneNumberForCheck === "")
        inputMethodHints: Qt.ImhDigitsOnly
//        errorHighlight: !text || errorOccured
        placeholderText: qsTr("Enter phone number")
        text: label
        onActiveFocusChanged: {
            if (text === label) {
                text = ""
            }
        }

        property string label: qsTr("Phone number")
//        EnterKey.iconSource: "image://theme/icon-m-enter-accept"
//        EnterKey.onClicked: {
//            authHelper.checkPhone(text)
//        }
        property bool errorOccured: {
            if (telegramCommonColumn.phoneRegistrationStatus !== registrationStatus.error) {
                return false
            }

            switch (telegramCommonColumn.authError) {
            case TelegramNamespace.AuthSignErrorPhoneNumberIsInvalid:
                return true;
            default:
                return false;
            }
        }

        onErrorOccuredChanged: {
            if (errorOccured) {
                focus = true
            }
        }
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        Button {
            id: checkPhoneButton
            visible: phoneNumberField.text !== ""
            enabled: phoneNumberField.enabled && !phoneNumberField.readOnly
            text: qsTr("Continue")
            onClicked: {
                authHelper.checkPhone(phoneNumberField.text)
            }

            Connections {
                target: authHelper
                onPendingPhoneNumberForCheckChanged: {
                    checkPhoneButton.text = qsTr("Check phone status")
                }
            }
        }
        Item {
            id: checkPhoneButtonSpacer
            visible: !checkPhoneButton.visible
            width: 1
            height: checkPhoneButton.height
        }
        Item {
            id: checkPhoneButtonPadding
            width: 1
            height: Theme.paddingSmall
        }
    }

    Column {
        id: detailsItem
        visible: telegramCommonColumn.showDetails
        width: parent.width

        Row {
            x: Theme.horizontalPageMargin
            spacing: Theme.paddingSmall
            height: phoneCodeBusyIndicator.height + Theme.paddingSmall
            Label {
                id: phoneStatusTextLabel
                text: qsTr("Phone status:")
                anchors.verticalCenter: parent.verticalCenter
            }
            BusyIndicator {
                id: phoneCodeBusyIndicator
//                size: BusyIndicatorSize.ExtraSmall
                running: authHelper.pendingPhoneNumberForCheck
                visible: running
                anchors.verticalCenter: phoneStatusTextLabel.verticalCenter

            }
            Label {
                id: phoneStatusLabel
                visible: !authHelper.pendingPhoneNumberForCheck
                text: {
                    switch (telegramCommonColumn.phoneRegistrationStatus) {
                    case registrationStatus.unknown:
                    default:
                        return qsTr("Unknown");
                    case registrationStatus.notRegistered:
                        return qsTr("Not registered");
                    case registrationStatus.registered:
                        return qsTr("Registered");
                    case registrationStatus.error:
                        switch (telegramCommonColumn.authError) {
                        case TelegramNamespace.AuthSignErrorAppIdIsInvalid:
                            return qsTr("App id invalid");
                        case TelegramNamespace.AuthSignErrorPhoneNumberIsInvalid:
                            return qsTr("The number is not valid");
                        default:
                            return qsTr("Unknown error");
                        }
                    }
                }

                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Column {
            visible: telegramCommonColumn.phoneRegistrationStatus === registrationStatus.registered
                     || telegramCommonColumn.phoneRegistrationStatus === registrationStatus.notRegistered
            width: parent.width

            Item {
                id: signButtonsPadding
                width: 1
                height: Theme.paddingSmall
            }

            Button {
                text: telegramCommonColumn.phoneRegistrationStatus === registrationStatus.registered ? qsTr("Sign in") : qsTr("Sign up")
                onClicked: {
                    authHelper.requestAuthCode(telegramCommonColumn.phoneNumber)
                }
                anchors.horizontalCenter: parent.horizontalCenter
            }

            TextField {
                id: authCodeField
                width: parent.width
                enabled: authHelper.authCodeNeeded
                onEnabledChanged: {
                    if (enabled) {
                        focus = true
                    }
                }
//                errorHighlight: authHelper.authCodeNeeded && !text
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData | Qt.ImhDigitsOnly
                echoMode: TextInput.Password
                property string label: qsTr("Auth code")
                text: activeFocus ? "" : label
                placeholderText: authHelper.authCodeNeeded ? label : "Auth code is not required (yet)"
//                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
//                EnterKey.onClicked: {
//                    authHelper.trySignIn(phoneNumberField.text, authCodeField.text)
//                }
            }

            TextField {
                id: passwordField
                width: parent.width
                enabled: authHelper.passwordNeeded
                onEnabledChanged: {
                    if (enabled) {
                        focus = true
                    }
                }
//                errorHighlight: authHelper.passwordNeeded && !text
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                echoMode: TextInput.Password
                text: qsTr("Password") // label
                placeholderText: authHelper.passwordNeeded ? label : qsTr("Password is not required (yet)")
//                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
//                EnterKey.onClicked: {
//                    authHelper.tryPassword2(text)
//                }
            }
        }

//        SectionHeader {
//            text: qsTr("Credentials data")
//        }

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width - Theme.horizontalPageMargin * 2
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeMedium
            color: Theme.highlightColor
            text: secretHelper.credentialDataExists ? "Credentials data already exists" : "Sign in and dump credentials data to create account"
        }

        Row {
            spacing: Theme.paddingLarge
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                enabled: authHelper.hasValidCredentials
                text: qsTr("Dump the data")
                onClicked: {
                    if(secretHelper.saveCredentialsData(telegramCore.connectionSecretData)) {
                        debugDataModel.addMessage("Credentials data saved")
                    } else {
                        debugDataModel.addMessage("Unable to save credentials data")
                    }
                }
            }
            Button {
                text: qsTr("Wipe exists data")
                enabled: secretHelper.credentialDataExists
                onClicked: {
                    if(secretHelper.removeCredentialsData()) {
                        debugDataModel.addMessage("Credentials data removed")
                    } else {
                        debugDataModel.addMessage("Unable to remove credentials data")
                    }
                }
            }
        }

    }
}
