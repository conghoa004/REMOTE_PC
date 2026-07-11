import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import "components"

Window {
    id: mainWindow
    visible: true
    width: 850
    height: 500
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2
    title: "Remote Desktop Connection"
    color: "#0A0A0B"

    // Quản lý trạng thái Bật/Tắt Host toàn cục
    property bool isHostEnabled: false

    // Quản lý cửa sổ điều khiển Remote
    property var controlWindow: null

    // Định nghĩa các tín hiệu (Signals) ở cấp độ Window
    signal connectRequested(string targetIp, string password)
    signal refreshCredentialsRequested()
    signal hostStateChanged(bool enabled)

    // Khai báo Toast toàn cục
    Toast {
        id: globalToast
        z: 999 // Đảm bảo luôn hiển thị trên cùng
    }

    // Khai báo các component backend (Đặt ngoài ScrollView để quản lý tốt hơn)
    NetworkManager {
        id: network
    }

    ConnectionManager {
        id: connection

        onStateChanged: {
            console.log("Connection State Changed:", state)
            if (state === ConnectionManager.Connected) {
                var component = Qt.createComponent("Control.qml")
                if (component.status === Component.Ready) {
                    mainWindow.controlWindow = component.createObject(mainWindow, { "connection": connection })
                    mainWindow.controlWindow.closing.connect(function() {
                        mainWindow.show()
                        if (connection.connected) {
                            connection.disconnectFromHost()
                        }
                        mainWindow.controlWindow = null
                    })
                    mainWindow.controlWindow.show()
                    mainWindow.hide()
                } else {
                    console.error("Error loading Control.qml:", component.errorString())
                    globalToast.show("Failed to open control screen: " + component.errorString())
                }
            } else if (state === ConnectionManager.Connecting) {
                globalToast.show("Connecting to " + targetIpInput.text + "...")
            } else if (state === ConnectionManager.Authenticating) {
                globalToast.show("Authenticating password...")
            } else if (state === ConnectionManager.Disconnected) {
                if (mainWindow.controlWindow !== null) {
                    mainWindow.controlWindow.close()
                    mainWindow.controlWindow = null
                }
                mainWindow.show()
            }
        }

        onClientConnected: {
            mainWindow.hide()
        }

        onClientDisconnected: {
            mainWindow.show()
        }

        onLastErrorChanged: {
            if (lastError !== "") {
                globalToast.show("Error: " + lastError)
            }
        }
    }

    ClipboardManager {
        id: clipboard
    }

    FontLoader {
        id: homeIconFont
        source: "../fonts/as7.otf"
    }

    // Giao diện chính bọc trong ScrollView để hỗ trợ màn hình nhỏ
    ScrollView {
        id: homeRoot
        anchors.fill: parent
        contentWidth: availableWidth
        contentHeight: mainLayout.implicitHeight
        clip: true // Tránh các thành phần con tràn ra ngoài khi scroll

        ColumnLayout {
            id: mainLayout
            width: parent.width
            anchors.margins: 32
            // Sử dụng các neo (anchors) thủ công kết hợp Layout để tránh xung đột anchors trong ScrollView
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            spacing: 28

            // ================= PHẦN 1: WELCOME BANNER =================
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true

                Text {
                    text: "Remote Desktop Connection"
                    color: "#F5F5F7"
                    font.pixelSize: 24
                    font.bold: true
                }

                Text {
                    text: "Securely connect to a remote computer or allow access to yours."
                    color: "#71717A"
                    font.pixelSize: 13
                }
            }

            // ================= PHẦN 2: GRID CONTROL PANELS =================
            RowLayout {
                Layout.fillWidth: true
                spacing: 24

                // --- BÊN TRÁI: THÔNG TIN MÁY NÀY (HOST) ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 250
                    radius: 12
                    color: "#18181A"
                    border.color: "#222224"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true

                            RowLayout {
                                spacing: 10
                                Text {
                                    font.family: homeIconFont.name
                                    text: "\uf109"
                                    color: mainWindow.isHostEnabled ? "#0A84FF" : "#52525B"
                                    font.pixelSize: 16
                                    Behavior on color { ColorAnimation { duration: 200 } }
                                }
                                Text {
                                    text: "This Computer (Host)"
                                    color: "#F5F5F7"
                                    font.pixelSize: 15
                                    font.bold: true
                                }
                            }

                            Item { Layout.fillWidth: true }

                            Switch {
                                id: hostSwitch
                                checked: connection.hosting
                                onClicked: {
                                    network.refresh()

                                    if (checked) {
                                        if (!connection.hosting) {
                                            var success = connection.startHost()
                                            if (!success) {
                                                globalToast.show("Failed to start host. Port may be in use.")
                                            }
                                        }
                                    } else {
                                        if (connection.hosting)
                                            connection.stopHost()
                                    }

                                    checked = Qt.binding(function() { return connection.hosting })
                                    mainWindow.isHostEnabled = connection.hosting
                                    mainWindow.hostStateChanged(connection.hosting)
                                    console.log("Host:", connection.hosting)
                                }

                                indicator: Rectangle {
                                    implicitWidth: 40
                                    implicitHeight: 22
                                    radius: 11
                                    color: hostSwitch.checked ? "#30D158" : "#27272A"
                                    border.color: hostSwitch.checked ? "#30D158" : "#3F3F46"
                                    border.width: 1
                                    Behavior on color { ColorAnimation { duration: 150 } }

                                    Rectangle {
                                        x: hostSwitch.checked ? 20 : 2
                                        y: 2
                                        width: 16
                                        height: 16
                                        radius: 8
                                        color: "#FFFFFF"
                                        Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            opacity: mainWindow.isHostEnabled ? 1.0 : 0.35
                            enabled: mainWindow.isHostEnabled
                            Behavior on opacity { NumberAnimation { duration: 200 } }

                            ColumnLayout {
                                spacing: 6
                                Layout.fillWidth: true
                                Text { text: "Your ID / IP Address"; color: "#71717A"; font.pixelSize: 11; font.bold: true }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 38
                                    color: "#222224"
                                    radius: 6

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12

                                        Text {
                                            id: id_host
                                            text: network.localIp
                                            color: "#FFFFFF"
                                            font.pixelSize: 14
                                            font.bold: true
                                            font.family: "Consolas"
                                            Layout.fillWidth: true
                                        }

                                        ItemDelegate {
                                            id: copyBtn
                                            Layout.preferredWidth: 24
                                            Layout.preferredHeight: 24
                                            background: Item {}
                                            Text {
                                                anchors.centerIn: parent
                                                font.family: homeIconFont.name
                                                text: "\uf0c5"
                                                color: copyBtn.hovered ? "#0A84FF" : "#52525B"
                                                font.pixelSize: 12
                                            }
                                            onClicked: clipboard.copy(id_host.text)
                                        }
                                    }
                                }
                            }

                            ColumnLayout {
                                spacing: 6
                                Layout.fillWidth: true
                                Text { text: "One-time Password"; color: "#71717A"; font.pixelSize: 11; font.bold: true }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 38
                                    color: "#222224"
                                    radius: 6

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12

                                        Text {
                                            text: mainWindow.isHostEnabled ? connection.password : "—"
                                            color: mainWindow.isHostEnabled ? "#30D158" : "#52525B"
                                            font.pixelSize: 14
                                            font.bold: true
                                            font.family: "Consolas"
                                            Layout.fillWidth: true
                                        }

                                        ItemDelegate {
                                            id: refreshBtn
                                            Layout.preferredWidth: 24
                                            Layout.preferredHeight: 24
                                            background: Item {}
                                            onClicked: {
                                                mainWindow.refreshCredentialsRequested()
                                                connection.generatePassword()
                                            }
                                            Text {
                                                anchors.centerIn: parent
                                                font.family: homeIconFont.name
                                                text: "\uf2f1"
                                                color: refreshBtn.hovered ? "#30D158" : "#52525B"
                                                font.pixelSize: 12
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Text {
                            text: mainWindow.isHostEnabled ? "● Status: Listening for incoming connections..." : "○ Status: Host is offline. Turn on to allow control."
                            color: mainWindow.isHostEnabled ? "#30D158" : "#71717A"
                            font.pixelSize: 11
                            font.italic: true
                            Layout.topMargin: 4
                            Behavior on color { ColorAnimation { duration: 200 } }
                        }
                    }
                }

                // --- BÊN PHẢI: KẾT NỐI ĐẾN MÁY KHÁC (CLIENT) ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 250
                    radius: 12
                    color: "#18181A"
                    border.color: "#222224"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 16

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            enabled: !mainWindow.isHostEnabled
                            opacity: mainWindow.isHostEnabled ? 0.35 : 1.0
                            Behavior on opacity { NumberAnimation { duration: 200 } }

                            RowLayout {
                                spacing: 10
                                Text {
                                    font.family: homeIconFont.name
                                    text: "\uf52e"
                                    color: mainWindow.isHostEnabled ? "#52525B" : "#0A84FF"
                                    font.pixelSize: 16
                                    Behavior on color { ColorAnimation { duration: 200 } }
                                }
                                Text {
                                    text: "Control Remote Computer"
                                    color: "#F5F5F7"
                                    font.pixelSize: 15
                                    font.bold: true
                                }
                            }

                            ColumnLayout {
                                spacing: 4
                                Layout.fillWidth: true
                                Text { text: "Partner ID / IP Address"; color: "#71717A"; font.pixelSize: 11; font.bold: true }

                                Rectangle {
                                    id: ipInputWrapper
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 38
                                    color: "#222224"
                                    radius: 6
                                    border.color: targetIpInput.activeFocus ? "#0A84FF" : "transparent"
                                    border.width: 1

                                    TextField {
                                        id: targetIpInput
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12
                                        placeholderText: "e.g. 192.168.1.100"
                                        placeholderTextColor: "#52525B"
                                        color: "#FFFFFF"
                                        font.pixelSize: 14
                                        font.family: "Consolas"
                                        verticalAlignment: TextInput.AlignVCenter
                                        background: Item {}
                                    }
                                }
                            }

                            ColumnLayout {
                                spacing: 4
                                Layout.fillWidth: true
                                Text { text: "Partner Password"; color: "#71717A"; font.pixelSize: 11; font.bold: true }

                                Rectangle {
                                    id: passwordInputWrapper
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 38
                                    color: "#222224"
                                    radius: 6
                                    border.color: targetPasswordInput.activeFocus ? "#0A84FF" : "transparent"
                                    border.width: 1

                                    property bool showPassword: false

                                    TextField {
                                        id: targetPasswordInput
                                        anchors.left: parent.left
                                        anchors.right: eyeButton.left
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 8
                                        placeholderText: "Enter password"
                                        placeholderTextColor: "#52525B"
                                        color: "#FFFFFF"
                                        font.pixelSize: 14
                                        font.family: "Consolas"
                                        echoMode: passwordInputWrapper.showPassword ? TextInput.Normal : TextInput.Password
                                        verticalAlignment: TextInput.AlignVCenter
                                        background: Item {}
                                    }

                                    Rectangle {
                                        id: eyeButton
                                        anchors.right: parent.right
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.rightMargin: 12
                                        width: 24
                                        height: 24
                                        color: "transparent"
                                        radius: 4

                                        MouseArea {
                                            id: eyeMouseArea
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: passwordInputWrapper.showPassword = !passwordInputWrapper.showPassword
                                        }

                                        Text {
                                            anchors.centerIn: parent
                                            font.family: homeIconFont.name
                                            text: passwordInputWrapper.showPassword ? "\uf070" : "\uf06e"
                                            color: eyeMouseArea.containsMouse ? "#FFFFFF" : "#71717A"
                                            font.pixelSize: 14
                                        }
                                    }
                                }
                            }

                            Button {
                                id: connectButton
                                Layout.fillWidth: true
                                Layout.preferredHeight: 40
                                Layout.topMargin: 4
                                onClicked: {
                                    if (targetIpInput.text.trim() === "") {
                                        globalToast.show("Please enter partner IP address")
                                        return
                                    }
                                    if (targetPasswordInput.text.trim() === "") {
                                        globalToast.show("Please enter password")
                                        return
                                    }
                                    mainWindow.connectRequested(targetIpInput.text, targetPasswordInput.text)
                                    connection.connectToHost(targetIpInput.text, targetPasswordInput.text)
                                }

                                background: Rectangle {
                                    radius: 6
                                    color: mainWindow.isHostEnabled ? "#27272A" : (connectButton.pressed ? "#0066CC" : (connectButton.hovered ? "#3399FF" : "#0A84FF"))
                                    Behavior on color { ColorAnimation { duration: 100 } }
                                }

                                contentItem: Item {
                                    RowLayout {
                                        anchors.centerIn: parent
                                        spacing: 8

                                        Text {
                                            font.family: homeIconFont.name
                                            text: "\uf1e6"
                                            color: mainWindow.isHostEnabled ? "#52525B" : "#FFFFFF"
                                            font.pixelSize: 14
                                        }
                                        Text {
                                            text: mainWindow.isHostEnabled ? "Disabled while Hosting" : "Connect to Partner"
                                            color: mainWindow.isHostEnabled ? "#71717A" : "#FFFFFF"
                                            font.bold: true
                                            font.pixelSize: 13
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ================= PHẦN 3: TRẠNG THÁI HỆ THỐNG CHÌM =================
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 45
                radius: 8
                color: "#131314"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 8

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: mainWindow.isHostEnabled ? "#FF9500" : "#30D158"
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }

                    Text {
                        text: mainWindow.isHostEnabled ? "System is in Host Mode (Outgoing connections blocked)" : "Ready for connection (Secure Network)"
                        color: "#71717A"
                        font.pixelSize: 12
                    }
                }
            }
        }
    }
}